#include "Server.h"

#include <cassert>
#include <exception>
#include <iostream>
#include <sstream>

#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

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

Server::Server(): connection_queue(10), is_listening()
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

    int connection; // TODO:              socket_addr, addr_size --- to be stored somewhere?
    while ((connection = accept(socket_fd, nullptr, nullptr)) >= 0)
    {
        connection_queue.push(connection);
    }

    std::cout << "Error on incoming connection ... [errno: " << errno << "]" << std::endl;
}

void Server::stop()
{
    connection_queue.stop();
    shutdown_socket(socket_fd);
}

Server::Request Server::get_next_request()
{
    int connection;
    if (!connection_queue.pop(&connection)) {
        return Request();
    }

    const size_t buffer_len = 4096;
    char* buffer = new char[buffer_len];

    const char *method, *path;
    int bytes_parsed, minor_version;
    phr_header headers[100];
    size_t method_len, path_len, num_headers;
    
    unsigned long buffer_index = 0;

    while (1) {
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

    return Request(method, method_len, path, path_len, headers, num_headers);
}


Server::Request::Request(const char* method, size_t method_len, const char* path, size_t path_len, phr_header* headers, size_t num_headers)
    : method(method, method_len), path(path, path_len), is_valid(true)
{
    this->headers.reserve(num_headers);
    if (headers) {
        for (size_t i = 0; i < num_headers; i++) {
            this->headers.emplace(this->headers.begin() + i,
                Header{
                    std::string(headers[i].name, headers[i].name_len),
                    std::string(headers[i].value, headers[i].value_len)
                });
        }
    }
}