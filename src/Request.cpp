#include "Request.h"


Request::Request(int connection, const char* method, size_t method_len, const char* path, size_t path_len, phr_header* headers, size_t num_headers)
    : connection(connection), method(method, method_len), path(path, path_len)
{
    this->headers.reserve(num_headers);
    if (headers) {
        for (size_t i = 0; i < num_headers; i++) {
            this->headers[std::string(headers[i].name, headers[i].name_len)] = std::string(headers[i].value, headers[i].value_len);
        }
    }
}
