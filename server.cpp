#include "server.h"

using namespace std::chrono_literals;

Server::Server(uint16_t port)
    : Port(port)
{
    ListeningSocketFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (ListeningSocketFd == -1) {
        PRINT_PERROR_MESSAGE("Failed to open listening socket");
        exit(1);
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(Port);
    
    if (bind(ListeningSocketFd, (const sockaddr*)&addr, sizeof(addr)) == -1) {
        PRINT_PERROR_MESSAGE("Failed to bind listening socket");
        exit(1);
    }
    
    if (listen(ListeningSocketFd, 1) == -1) {
        PRINT_PERROR_MESSAGE("Failed to listen");
        exit(1);
    }
}

Server::~Server()
{
    if (close(ListeningSocketFd) == -1) {
        PRINT_PERROR_MESSAGE("Failed to close listening socket");
    }
}

void Server::Run()
{
    while (!IsSignalCaught) {
        int newSocketFd = accept(ListeningSocketFd, nullptr, nullptr);
        if (newSocketFd == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                PRINT_PERROR_MESSAGE("Failed to accept");
            }
            continue;
        } else {
            Connections.Add(newSocketFd);
        }
    }
}

