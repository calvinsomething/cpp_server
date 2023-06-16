#pragma once

#include <queue>
#include <atomic>
#include <condition_variable>

#include "atomic_queue.h"

class Server
{
    int socket_fd;
    const size_t buffer_size = 0;

    AtomicQueue<int, 100> connection_queue;
public:
    Server();
    ~Server();
    void start(unsigned port, unsigned queue_size, unsigned buffer_len);
    void stop();
    char* get_next_request();
};