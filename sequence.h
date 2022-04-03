#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>

class Sequence
{
public:
    Sequence() {}
    void Set(uint64_t base, uint64_t step);
    bool IsSet() const { return IsSet_; }
    uint64_t GetNextValue();

private:
    bool IsSet_ = false;
    uint64_t Base;
    uint64_t Step;
};

