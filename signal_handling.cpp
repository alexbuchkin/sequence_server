#include "signal_handling.h"

std::atomic_bool IsSignalCaught{false};

void HandleSignal(int signum)
{
    IsSignalCaught = true;
}
