#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const int PORT = 8080, bufferSize = 30000;

int main()
{
    int sfd;
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Could not create socket." << std::endl;
        exit(1);
    }

    sockaddr_in sa;
    socklen_t saLen = sizeof(sa);

    memset(reinterpret_cast<char*>(&sa.sin_zero), 0, sizeof(sa.sin_zero));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(8080);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sfd, reinterpret_cast<sockaddr*>(&sa), saLen) < 0)
    {
        std::cerr << "Could not bind socket address." << std::endl;
        exit(1);
    }

    if (listen(sfd, 10) < 0)
    {
        std::cerr << "Could not listen on socket address." << std::endl;
        exit(1);
    }

    int incSocket;
    while ((incSocket = accept(sfd, reinterpret_cast<sockaddr*>(&sa), &saLen)) >= 0)
    {
        char buffer[bufferSize] = {};
        read(incSocket , buffer, bufferSize);

        std::cout << buffer << std::endl;

        const char* msg = "It's working.";
        write(incSocket, msg, strlen(msg));

        close(incSocket);
    }

    std::cerr << "Error (" << incSocket << "): could not accept network request." << std::endl;
    exit(1);
}