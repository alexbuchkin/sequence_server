#pragma once

#include <atomic>
#include <csignal>

extern std::atomic_bool IsSignalCaught;

void HandleSignal(int signum);

