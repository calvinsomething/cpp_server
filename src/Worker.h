#pragma once

#include <thread>

#include "Request.h"
#include "Server.h"

typedef void (* Mux)(Request&);

class Worker : public std::thread {
    Server* server;
    Mux mux;
    
    void do_work();
public:
    Worker(Worker&& other) = default;

    Worker(Server* server, Mux mux)
        : std::thread([server, mux]() {
            Request request;
            while (request = server->get_next_request()) {
                // handle request (pass to multiplexing function)
                std::cout << "Got request: " << request.path << " --- " << request.method << std::endl;
                // ...
                mux(request);
            }
            // when empty request is sent, it means connections are closed/server has stopped
        })
    {
    }

    ~Worker()
    {
        std::thread::join();
    }
};