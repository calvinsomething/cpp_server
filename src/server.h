#pragma once

#include <queue>
#include <atomic>
#include <condition_variable>

#include "atomic_queue.h"

class Server
{
    int socket_fd;
    AtomicQueue<int, 100> connection_queue;
public:
    Server();
    ~Server();
    void start(unsigned port, unsigned queue_size);
    void stop();
    char* get_next_request();
};