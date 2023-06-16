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

std::string fmtSocketError(const char* action, int error_code)
{
    std::stringstream ss;
    ss << "Failed to " << action << " server socket. [error code: " << error_code << "]";
    return ss.str();
}

Server::Server(): connection_queue(10)
{
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        throw std::runtime_error(fmtSocketError("create", socket_fd));
    }
}

Server::~Server()
{
    close(socket_fd);
}

void Server::start(unsigned port, unsigned queue_size, unsigned buffer_len)
{
    // Prepare listening socket
    sockaddr_in address = {};

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    int result;
    if ((result = bind(socket_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address))) < 0)
    {
        throw std::runtime_error(fmtSocketError("bind", result));
    }

    if ((result = listen(socket_fd, queue_size)) < 0)
    {
        throw std::runtime_error(fmtSocketError("listen on", result));
    }

    std::cout << "Server listening on port " << port << "..." << std::endl;

    // Prepare for incoming connections
    sockaddr connection_address = {};
    socklen_t connection_size = sizeof(connection_address);

    int connection;
    const_cast<size_t&>(buffer_size) = buffer_len * sizeof(char);

    std::thread thread([this]() {
            std::cout << "Thread started..." << std::endl;
            // should use setsockopt() to make connection socket reusable/still-alive
            this->get_next_request(); // this is blocking --> need way to abort if server is stopped
            // should handle RAII when done with connection socket
            // request should be handed off to mux
    });

    while ((connection = accept(socket_fd, &connection_address, &connection_size)) >= 0)
    {
        connection_queue.push(connection);
    }

    thread.join();

    std::cout << "Error on incoming connection ... [errno: " << errno << "]" << std::endl;
}

void Server::stop()
{
    if (shutdown(socket_fd, 0)) {
        int err = errno;
        switch (err) {
        case EBADF:
            std::cout << "Socket is not a valid file descriptor." << std::endl;
        case ENOTSOCK:
            std::cout << "Socket is not a socket." << std::endl;
        case ENOTCONN:
            std::cout << "Socket is not connected." << std::endl;
        }
    }

    // have to close all connection sockets also...
}

char* Server::get_next_request()
{
    // blocks here waiting for connections to be put on queue -- need way to cancel if server is stopped
    int connection = connection_queue.pop();

    char* buffer = new char[buffer_size];
    // read(connection, buffer, buffer_size);

    char buf[4096];
    const char *method, *path;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
    ssize_t rret;

    while (1) {
        /* read the request */
        while ((rret = read(connection, buf + buflen, sizeof(buf) - buflen)) == -1 && errno == EINTR)
            ;
        if (rret <= 0)
            throw std::runtime_error("IOERROR");
        prevbuflen = buflen;
        buflen += rret;
        /* parse the request */
        num_headers = sizeof(headers) / sizeof(headers[0]);
        pret = phr_parse_request(buf, buflen, &method, &method_len, &path, &path_len,
                                &minor_version, headers, &num_headers, prevbuflen);
        if (pret > 0)
            break; /* successfully parsed the request */
        else if (pret == -1)
            throw std::runtime_error("ParseError");
        /* request is incomplete, continue the loop */
        assert(pret == -2);
        if (buflen == sizeof(buf))
            throw std::runtime_error("RequestIsTooLongError");
    }

    printf("request is %d bytes long\n", pret);
    printf("method is %.*s\n", (int)method_len, method);
    printf("path is %.*s\n", (int)path_len, path);
    printf("HTTP version is 1.%d\n", minor_version);
    printf("headers:\n");
    for (int i = 0; i != num_headers; ++i) {
        printf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
            (int)headers[i].value_len, headers[i].value);
    }

    std::cout << buf << std::endl;

    const char* msg = "It's working.";

    write(connection, msg, strlen(msg));
    close(connection);

    return buffer;
}
