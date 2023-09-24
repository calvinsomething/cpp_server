#pragma once

#include "pch.h"

#include "AtomicQueue.h"
#include "Request.h"
#include "ResponseWriter.h"

typedef void (* Handler)(Request, ResponseWriter);

struct History
{
    std::chrono::time_point<std::chrono::steady_clock> created_at;
    unsigned request_count;
};

class Server
{
    // TODO: update to atomic map
    std::unordered_map<int, History> open_connections;

    int listening_socket, epoll;
    AtomicQueue<int, 100> connection_queue;
    bool is_listening;
    
public:
    Server();
    ~Server();

    void start(unsigned port, unsigned queue_size);
    void stop(std::exception_ptr exception = nullptr);
    void unwatch(int connection);
    bool dispatch(Handler handler);
    void update_epoll_list(int connection, int operation, int flags);
    void remove_from_epoll(int fd);
};
