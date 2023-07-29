#pragma once

#include <string>
#include <vector>


class ResponseWriter
{
    int fd;
public:
    ResponseWriter(int fd): fd(fd) {}
    void write_text(std::string text);
    void write_file(std::string file_path);
};
