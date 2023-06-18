#include "server.h"

#include <cassert>
#include <exception>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

extern "C" {
    #include <picohttpparser/picohttpparser.h>
}

// helpers

std::runtime_error socket_setup_error(const char* action, int error_code)
{
    std::stringstream ss;
    ss << "Failed to " << action << " server socket. [error code: " << error_code << "]";
    return std::runtime_error(ss.str());
}

void shutdown_socket(int socket_fd)
{
    if (shutdown(socket_fd, SHUT_RDWR)) {
        int err = errno;
        switch (err) {
        case EBADF:
            std::cout << "'" << socket_fd << "' is not a valid file descriptor." << std::endl;
        case ENOTSOCK:
            std::cout << "fd[" << socket_fd << "] is not a socket." << std::endl;
        case ENOTCONN:
            std::cout << "Socket (fd[" << socket_fd << "]) is not connected." << std::endl;
        }
    }
}

// Server implementation

Server::Server(): connection_queue(10)
{
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        throw socket_setup_error("create", socket_fd);
    }
}

Server::~Server()
{
    close(socket_fd);
}

void Server::start(unsigned port, unsigned queue_size)
{
    // Prepare listening socket
    sockaddr_in address = {};

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    int result;
    if ((result = bind(socket_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address))) < 0)
    {
        throw socket_setup_error("bind", result);
    }

    if ((result = listen(socket_fd, queue_size)) < 0)
    {
        throw socket_setup_error("listen on", result);
    }

    std::cout << "Server listening on port " << port << "..." << std::endl;

    // Prepare for incoming connections
    sockaddr connection_address = {};
    socklen_t connection_size = sizeof(connection_address);

    int connection;

    std::thread thread([this]() {
            // should use setsockopt() to make connection socket reusable/still-alive
            // then store open connections somewhere to be shutdown/closed when server is stopped
            this->get_next_request();
    });

    while ((connection = accept(socket_fd, nullptr, nullptr)) >= 0)
    {
        // if connection_address / address_size are needed, will need to pass a struct to the connection queue
        connection_queue.push(connection);
    }
    thread.join();

    std::cout << "Error on incoming connection ... [errno: " << errno << "]" << std::endl;
}

void Server::stop()
{
    connection_queue.stop();
    shutdown_socket(socket_fd);
}

// should pass parsed request as struct to mux
char* Server::get_next_request()
{
    int connection;
    if (!connection_queue.pop(&connection)) {
        return nullptr;
    }

    char* buffer = new char[4096];
    const int buffer_size = 4096 * sizeof(char);

    const char *method, *path;
    int bytes_parsed, minor_version;
    struct phr_header headers[100];
    size_t method_len, path_len, num_headers;
    
    unsigned long buffer_index = 0;

    while (1) {
        long bytes_read;
        while ((bytes_read = read(connection, buffer + buffer_index, buffer_size - buffer_index)) == -1 && errno == EINTR);
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

    printf("request is %d bytes long\n", bytes_parsed);
    printf("method is %.*s\n", (int)method_len, method);
    printf("path is %.*s\n", (int)path_len, path);
    printf("HTTP version is 1.%d\n", minor_version);
    printf("headers:\n");
    for (int i = 0; i != num_headers; ++i) {
        printf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
            (int)headers[i].value_len, headers[i].value);
    }

    std::cout << buffer << std::endl;

    const char* msg = "It's working.";

    write(connection, msg, strlen(msg));
    close(connection);

    return buffer;
}
