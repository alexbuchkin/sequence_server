#pragma once

#include <cctype>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <unistd.h>
#include <unordered_map>

#include "client_handler.h"
#include "pipe.h"
#include "signal_handling.h"

class ConnectionHolder
{
public:
    ConnectionHolder();
    ~ConnectionHolder();
    void Add(int socketFd);

private:
    uint64_t Counter = 0;
    std::unordered_map<uint64_t, std::future<void>> Storage;
    std::mutex Mutex;
    Pipe Pipe_;
    std::future<void> InterruptedConnectionsCleaner;
};

