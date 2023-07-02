#pragma once

#include <thread>

#include "Mux.h"
#include "Server.h"

class Worker : public std::thread {
public:
    Worker(Worker&& other) = default;

    Worker(Server* server)
        : std::thread([server]()
        {
                while (server->dispatch(mux)) // when dispatch returns false, connections have been closed
                {
                    std::cout << "Handled request....." << std::endl;
                }
        })
    {
    }

    ~Worker()
    {
        std::thread::join();
    }
};