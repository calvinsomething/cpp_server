#pragma once

#include <vector>

#include <picohttpparser/picohttpparser.h>

#include "AtomicQueue.h"

class Server
{
    int socket_fd;
    AtomicQueue<int, 100> connection_queue;
    bool is_listening;
public:
    class Request {
        bool is_valid;
    public:
        operator bool () {
            return is_valid;
        }

        struct Header {
            std::string name;
            std::string value;
        };

        std::string method;
        std::string path;
        std::vector<Header> headers;

        Request(): is_valid(false) {};
        Request(const char* method, size_t method_len, const char* path, size_t path_len, phr_header* headers, size_t num_headers);
    };

    Server();
    ~Server();

    void start(unsigned port, unsigned queue_size);
    void stop();
    
    Request get_next_request();
};