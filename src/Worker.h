#pragma once

#include "pch.h"

#include "mux.h"
#include "Server.h"
#include "utils.h"

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
                    return;
                }
                std::stringstream ss;
                ss << "Thread " << std::this_thread::get_id() << " exitted gracefully." << std::endl;
                std::cout << ss.str() << std::endl;
            }
        )
    {
    }

    ~Worker()
    {
        std::thread::join();
    }
};
