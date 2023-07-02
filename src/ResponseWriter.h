#pragma once

#include <string>
#include <vector>


class ResponseWriter
{
    int fd;
public:
    ResponseWriter(int fd): fd(fd) {}
    void write_text(std::string text);
};
