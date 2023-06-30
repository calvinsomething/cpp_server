#include "Request.h"


Request::Request(const char* method, size_t method_len, const char* path, size_t path_len, phr_header* headers, size_t num_headers)
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