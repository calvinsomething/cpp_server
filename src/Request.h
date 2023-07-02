#pragma once

#include <string>
#include <vector>

#include <picohttpparser/picohttpparser.h>

class Request {
    int fd;
public:
    struct Header {
        std::string name;
        std::string value;
    };

    std::string method;
    std::string path;
    std::vector<Header> headers;

    Request(int fd, const char* method, size_t method_len, const char* path, size_t path_len, phr_header* headers, size_t num_headers);
};