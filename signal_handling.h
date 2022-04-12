#pragma once

#include <atomic>
#include <csignal>

/*
 * True if SIGINT was caught
 */
extern std::atomic_bool IsSignalCaught;

void HandleSignal(int signum);

