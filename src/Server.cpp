#include "Server.h"

#include <cassert>
#include <exception>
#include <iostream>
#include <sstream>
#include <thread>
#include <utility>

#include <errno.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Helpers

std::runtime_error socket_setup_error(const char* action, int error_code)
{
    std::stringstream ss;
    ss << "Failed to " << action << " server socket. [error code: " << error_code << "]";
    return std::runtime_error(ss.str());
}

bool ci_str_equal(const char* a, const char* b, const unsigned n)
{
    for (int i = 0; i < n; i++)
    {
        if (*a != *b && *a + 32 != *b && *a - 32 != *b) {
            return false;
        }
    }
    return true;
}

// Server implementation

Server::Server(): connection_queue(10), is_listening()
{
    if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        throw socket_setup_error("create", listening_socket);
    }
}

Server::~Server()
{
    close(listening_socket);
}

void Server::start(unsigned port, unsigned queue_size)
{
    // Prepare listening socket
    sockaddr_in address = {};

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    int result;
    if ((result = bind(listening_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address))) < 0)
    {
        throw socket_setup_error("bind", result);
    }

    if ((result = listen(listening_socket, queue_size)) < 0)
    {
        throw socket_setup_error("listen on", result);
    }

    std::cout << "Server listening on port " << port << "..." << std::endl;

    epoll = epoll_create1(0);

    epoll_event input_event;
    input_event.events = EPOLLIN;
    input_event.data.fd = listening_socket;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, listening_socket, &input_event))
    {
        throw std::runtime_error("Error registering polling socket.");
    }

    int n;
    epoll_event connection_event;
    while (n = epoll_wait(epoll, &connection_event, 1, -1))
    {
        if (n == -1) {
            perror("epoll_wait");
            break;
        }
        int connection;
        if (connection_event.data.fd == listening_socket) {
            if ((connection = accept(listening_socket, nullptr, nullptr)) < 0)
            {
                perror("accepting connection");
                break;
            }
        } else {
            connection = connection_event.data.fd;
        }
        connection_queue.push(connection);
    }
}

void Server::stop()
{
    connection_queue.stop();
    if (shutdown(listening_socket, SHUT_RDWR))
    {
        perror("shutting down listening socket");
    }
}

bool Server::dispatch(Handler handler)
{
    int connection;
    if (!connection_queue.pop(&connection))
    {
        return false;
    }

    const size_t buffer_len = 4096;
    char* buffer = new char[buffer_len];

    const char *method, *path;
    int bytes_parsed, minor_version;
    phr_header headers[100];
    size_t method_len, path_len, num_headers;
    
    unsigned long buffer_index = 0;

    while (1)
    {
        long bytes_read;
        while ((bytes_read = read(connection, buffer + buffer_index, buffer_len - buffer_index)) == -1 && errno == EINTR);
        if (bytes_read <= 0)
        {
            throw std::runtime_error("IOERROR");
        }

        auto prev_index = buffer_index;
        buffer_index += bytes_read;

        num_headers = sizeof(headers) / sizeof(headers[0]);

        bytes_parsed = phr_parse_request(buffer, buffer_index, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers, prev_index);
        
        if (bytes_parsed > 0)
        {
            break;
        }
        else if (bytes_parsed == -1)
        {
            throw std::runtime_error("ParseError");
        }
        assert(bytes_parsed == -2);

        if (buffer_index == sizeof(buffer))
        {
            throw std::runtime_error("Request is too long.");
        }
    }

    for (int i = 0; i < num_headers; i++)
    {
        if (headers[i].name_len == 10 && ci_str_equal(headers[i].name, "Connection", 10))
        {
            if (headers[i].value_len == 10 && ci_str_equal(headers[i].value, "Keep-Alive", 10))
            {
                keep_alive(connection);
            }
            break;
        }
    }

    handler(
        Request(connection, method, method_len, path, path_len, headers, num_headers),
        ResponseWriter(connection)
    );
    return true;
}

void Server::keep_alive(int connection)
{
    epoll_event input_event;
    input_event.events = EPOLLIN;
    input_event.data.fd = connection;

    // move to ConnectionManager -- only add to epoll if connection not already managed
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, connection, &input_event))
    {
        perror("error adding keep-alive connection to epoll");
    }
    // register connection with ConnectionManager -- manage keep-alive timeout/max
}