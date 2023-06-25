#pragma once

#include <thread>

#include "Server.h"
#include "Worker.h"

class Mux {
    Server& server;
    std::vector<Worker> workers;
public:
    Mux(Server& server, size_t worker_count);
};