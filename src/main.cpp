#include <iostream>
#include <exception>

#include <signal.h>

#include "Mux.h"
#include "Server.h"

const int PORT = 8080, REQUEST_QUEUE_SIZE = 10;

// Create server -- make global/static so it can be stopped in sigint_handler
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
    // TODO: should update to sigaction
    signal(SIGINT, sigint_handler);
    try {
        Mux mux(server, 4);

        server.start(PORT, REQUEST_QUEUE_SIZE);
    } catch (std::exception err) {
        std::cout << err.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown error." << std::endl;
    }
}
