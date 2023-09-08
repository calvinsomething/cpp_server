#pragma once

#include <exception>
#include <iostream>
#include <thread>

#include "mux.h"
#include "Server.h"

class Worker : public std::thread
{
public:
    Worker(Worker&& other) = default;

    Worker(Server* server)
        : std::thread(
            [server]()
            {
                try
                {
                    while (server->dispatch(mux)) // when dispatch returns false, connections have been closed
                    {
                        std::cout << "Handled request....." << std::endl;
                    }
                }
                catch (...)
                {
                    server->stop(std::current_exception());
                }
            }
        )
    {
    }

    ~Worker()
    {
        std::thread::join();
    }
};
