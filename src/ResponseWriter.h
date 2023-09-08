#pragma once

#include <map>
#include <string>

class ResponseWriter
{
    int fd;
    bool keep_alive;

    std::map<std::string, std::string> headers;

    void send_headers(unsigned status, int send_flags);
public:
    ResponseWriter(int fd): fd(fd), keep_alive(true) {}

    void no_content(unsigned status);

    void send_text(unsigned status, std::string text);
    void send_file(unsigned status, std::string file_path);

    void set_header(std::string name, std::string value);
    void remove_header(std::string name);

    void set_keep_alive(bool value);

    void close();
};
