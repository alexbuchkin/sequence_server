#pragma once

#include <cctype>
#include <mutex>
#include <queue>

struct Pipe
{
    std::queue<uint64_t> Queue;
    std::mutex Mutex;
};

