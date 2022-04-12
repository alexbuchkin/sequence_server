#pragma once

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <future>
#include <list>
#include <netdb.h>
#include <sys/socket.h>

#include "client_handler.h"
#include "error_message.h"
#include "signal_handling.h"

class Server
{
public:
    Server(uint16_t port);
    ~Server();
    void Run();
    
private:
    void RemoveInterruptedConnections();
    
private:
    uint16_t Port;
    int ListeningSocketFd;
    std::list<std::future<void>> Connections;
};
