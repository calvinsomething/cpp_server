#pragma once

#include <thread>

#include "Server.h"

class Worker : public std::thread {
public:
    static void do_work(Server* server);
    Worker(Worker&& other) = default;
    Worker(Server* server);
    ~Worker();
};