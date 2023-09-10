#include "ResponseWriter.h"

#include <cassert>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <unordered_map>

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "utils.h"

extern std::unordered_map<unsigned, const char*> STATUSES;

// Required Headers:
//  date: Tue, 05 Sep 2023 15:18:53 GMT
//  [in 405 Method Not Allowed Resp] Allow: GET, HEAD, PUT
//  [in 401] WWW-Authenticate...

void ResponseWriter::no_content(unsigned status)
{
    send_headers(status, 0);
}

void ResponseWriter::send_text(unsigned status, std::string text)
{
    set_header("Content-Length", std::to_string(text.size()));
    send_headers(status, MSG_MORE);
    send(fd, static_cast<const void*>(text.c_str()), text.size(), 0);
}

void ResponseWriter::send_file(unsigned status, std::string file_path)
{
    send_headers(status, MSG_MORE);
    // TODO
    // use sendfile()
}

void ResponseWriter::close()
{
    if (::close(fd))
    {
        std::unique_ptr<char[]> err_msg = f_str("Error closing connection '%d' with ERRNO '%d'", 10, fd, errno);
        perror(err_msg.get());
        delete err_msg.get();
    }
}

void ResponseWriter::set_header(std::string name, std::string value)
{
    assert(!has_white_space(name.c_str()));
    assert(name.size());
    assert(value.size());
    assert(!ci_str_equal(name.c_str(), "Date", comptime_len("Date")));
    assert(!ci_str_equal(name.c_str(), "Connection", comptime_len("Connection")));
    headers[name] = value;
}

void ResponseWriter::remove_header(std::string name)
{
    headers.erase(name);
}

void ResponseWriter::send_headers(unsigned status, int send_flags)
{
    std::stringstream ss;
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    ss << "HTTP/1.1 " << status << " " << STATUSES.at(status) << "\nDate: " << std::put_time(gmtime(&now), "%a, %d %b %Y %H:%M:%S GMT");

    if (!keep_alive)
    {
        ss << "Connection: close";
    }

    for (auto& [name, value] : headers)
    {
        ss << "\n" << name << ": " << value;
    }
    ss << "\n\n";

    std::string str = ss.str();

    send(fd, static_cast<const void*>(str.c_str()), str.size(), send_flags);
}
