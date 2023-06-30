#pragma once

#include <vector>

#include <picohttpparser/picohttpparser.h>

#include "AtomicQueue.h"
#include "Request.h"

class Server
{
    int socket_fd;
    AtomicQueue<int, 100> connection_queue;
    bool is_listening;
    
public:
    Server();
    ~Server();

    void start(unsigned port, unsigned queue_size);
    void stop();
    
    Request get_next_request();
};