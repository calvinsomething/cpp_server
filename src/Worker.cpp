#include "Worker.h"

#include <iostream>

void Worker::do_work(Server* server) {
    Server::Request request;
    while (request = server->get_next_request()) {
        // handle request (pass to multiplexing function)
        std::cout << "Got request: " << request.path << " --- " << request.method << std::endl;
        // ...
    }
    // when empty request is sent, it means connections are closed/server has stopped
}

Worker::Worker(Server* server) : std::thread(Worker::do_work, server) {}

Worker::~Worker() {
    std::thread::join();
}