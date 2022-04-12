#pragma once

#include <cctype>
#include <mutex>
#include <queue>

/* For use as a UNIX Pipe to communicate between threads
 *
 */
struct Pipe
{
    std::queue<uint64_t> Queue;
    std::mutex Mutex;
};

