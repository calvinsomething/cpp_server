#pragma once

#include "AtomicQueue.h"
#include "Request.h"
#include "ResponseWriter.h"

typedef void (* Handler)(Request, ResponseWriter);

class Server
{
    int listening_socket, epoll;
    AtomicQueue<int, 100> connection_queue;
    bool is_listening;
    
public:
    Server();
    ~Server();

    void start(unsigned port, unsigned queue_size);
    void stop();
    bool dispatch(Handler handler);
    void keep_alive(int connection);
};