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
    /*
     * Launch InterruptedConnectionsCleaner
     */
    ConnectionHolder();

    /*
     * Run wait() for every future in Storage
     */
    ~ConnectionHolder();

    /*
     * Construct ClientHandler and run it
     * in separate thread, then push received std::future
     * into Storage.
     * Every ClientHandler instance will obtain
     * unique ID (current Counter's value)
     */
    void Add(int socketFd);

private:
    /*
     * Counter for ClientHandler's ID distributing
     */
    uint64_t Counter = 0;

    std::unordered_map<uint64_t, std::future<void>> Storage;

    /*
     * For thread-safety when working with Storage
     */
    std::mutex Mutex;

    /*
     * Pipe for receiving interrupted connection IDs
     */
    Pipe Pipe_;

    /*
     * Launches in separate thread.
     * Reads all interrupted connection ids from Pipe_
     * then removes these connections from Storage in eternal cycle.
     * Sleeps for 1 second every iteration
     */
    std::future<void> InterruptedConnectionsCleaner;
};

