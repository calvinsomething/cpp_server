#include <iostream>
#include <exception>

#include "signal.h"

#include "server.h"

const int PORT = 8080, REQUEST_QUEUE_SIZE = 10;

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
    // should update to sigaction
    signal(SIGINT, sigint_handler);
    try {
        new(&server) Server;
        std::cout << "Server created..." << std::endl;

        server.start(PORT, REQUEST_QUEUE_SIZE);
    } catch (std::exception err) {
        std::cout << err.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown error." << std::endl;
    }
}
