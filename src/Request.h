#pragma once

#include "pch.h"

class Request
{
    int connection;
public:
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;

    Request(int connection, const char* method, size_t method_len, const char* path, size_t path_len, phr_header* headers, size_t num_headers);
};
