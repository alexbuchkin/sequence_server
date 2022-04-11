#include "sequence.h"

    
void Sequence::Set(uint64_t base, uint64_t step)
{
    if (IsSet()) {
        throw std::runtime_error("Sequence is already set");
    }
    
    IsSet_ = true;
    Base = CurrentValue = base;
    Step = step;
}

uint64_t Sequence::GetNextValue()
{
    const uint64_t current_value = CurrentValue;
    const uint64_t diff = std::numeric_limits<uint64_t>::max() - CurrentValue;
    if (Step - 1 >= diff) {
        CurrentValue = Base;
    } else {
        CurrentValue += Step;
    }
    
    return current_value;
}
