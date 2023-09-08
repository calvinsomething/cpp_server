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

#include "utils.h"

const int MAX_REQUESTS = 100, TIMEOUT = 5;

// Helpers

std::runtime_error socket_setup_error(const char* action, int error_code)
{
    std::stringstream ss;
    ss << "Failed to " << action << " server socket. [error code: " << error_code << "]";
    return std::runtime_error(ss.str());
}

// Server implementation

Server::Server(): connection_queue(10), is_listening()
{
    if ((listening_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0)
    {
        throw socket_setup_error("create", listening_socket);
    }
}

Server::~Server()
{
    close(listening_socket);
}

void Server::add_to_epoll(int fd)
{
    epoll_event input_event;
    input_event.events = EPOLLIN;
    input_event.data.fd = fd;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &input_event))
    {
        perror("Error registering polling socket:");
        exit(EXIT_FAILURE);
    }
}

void Server::remove_from_epoll(int fd)
{
    if (epoll_ctl(epoll, EPOLL_CTL_DEL, fd, 0))
    {
        perror("Error removing expired connection from epoll list:");
        exit(EXIT_FAILURE);
    }
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
    add_to_epoll(listening_socket);

    int n;
    epoll_event connection_event;
    while ((n = epoll_wait(epoll, &connection_event, 1, -1)))
    {
        if (n == -1) {
            perror("error returned from epoll_wait");
            break;
        }
        int conn_fd;
        if (connection_event.data.fd == listening_socket) {
            if ((conn_fd = accept(listening_socket, nullptr, nullptr)) < 0)
            {
                perror("error accepting conn_fd");
                break;
            }
        } else {
            conn_fd = connection_event.data.fd;
        }
        connection_queue.push(conn_fd);
    }
}

void Server::stop(std::exception_ptr exception)
{
    connection_queue.stop();
    if (shutdown(listening_socket, SHUT_RDWR))
    {
        perror("error shutting down listening socket");
    }
    if (exception)
    {
        std::rethrow_exception(exception);
    }
}

bool Server::dispatch(Handler handler)
{
    int connection;
    if (!connection_queue.pop(&connection))
    {
        return false;
    }
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    ResponseWriter response_writer(connection);

    auto connection_history = open_connections.find(connection);
    bool is_open = false;

    if (connection_history != open_connections.end())
    {
        if (
            // time expired
            std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - connection_history->second.created_at) > std::chrono::seconds(TIMEOUT)
            // max requests exceeded
            || connection_history->second.request_count == MAX_REQUESTS
        )
        {
            response_writer.set_keep_alive(false);
            remove_from_epoll(connection);
            open_connections.erase(connection);
        }
        else
        {
            connection_history->second.request_count++;
        }
        is_open = true;
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
        if (headers[i].name_len == 10 && ci_str_equal(headers[i].name, "connection", 10))
        {
            if (headers[i].value_len == 10 && ci_str_equal(headers[i].value, "close", 5))
            {
                // can read request, but should not respond
                response_writer.close();
            }
            break;
        }
    }

    if (!is_open)
    {
        add_to_epoll(connection);
        open_connections[connection] = {timestamp, 1};
    }

    handler(
        Request(connection, method, method_len, path, path_len, headers, num_headers),
        response_writer
    );
    return true;
}
