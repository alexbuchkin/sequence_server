#pragma once

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <future>
#include <list>
#include <netdb.h>
#include <sys/socket.h>

#include "connection_holder.h"
#include "error_message.h"
#include "signal_handling.h"

class Server
{
public:
    /*
     * Opens listening socket
     */
    Server(uint16_t port);

    /*
     * Closes listening socket
     */
    ~Server();

    /*
     * Tries to call accept from listening socket
     * and to obtain new connection
     * in eternal cycle
     */
    void Run();

private:
    uint16_t Port;
    int ListeningSocketFd;
    ConnectionHolder Connections;
};
