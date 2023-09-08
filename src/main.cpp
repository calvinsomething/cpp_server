#include <exception>
#include <iostream>
#include <vector>

#include <signal.h>

#include "mux.h"
#include "Server.h"
#include "Worker.h"

const int PORT = 8080, REQUEST_QUEUE_SIZE = 10, WORKER_COUNT = 4;

// Create server -- make static so it can be stopped in sigint_handler
static Server server;

extern "C" {
    void sigint_handler(int signal)
    {
        server.stop();
        std::cout << "Server stopped." << std::endl;
    }
}

int main()
{
    struct sigaction s{};
    s.sa_handler = sigint_handler;
    s.sa_flags = SA_RESETHAND;
    sigaction(SIGINT, &s, nullptr);

    try {
        std::vector<Worker> workers;
        workers.reserve(WORKER_COUNT);
        for (size_t i = 0; i < WORKER_COUNT; i++)
        {
            workers.emplace_back(&server);
        }

        server.start(PORT, REQUEST_QUEUE_SIZE);
    } catch (std::exception err) {
        std::cout << err.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown error." << std::endl;
    }
}
