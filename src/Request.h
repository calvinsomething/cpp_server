#pragma once

#include <string>
#include <vector>

#include <picohttpparser/picohttpparser.h>

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