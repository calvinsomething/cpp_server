#include "Mux.h"

#include<iostream>

#include "Server.h"

Mux::Mux(Server& server, size_t worker_count) : server(server) {
    workers.reserve(worker_count);
    
    for (size_t i = 0; i < worker_count; i++) {
        workers.emplace_back(&server);
    }
}