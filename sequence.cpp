#include "sequence.h"

	
void Sequence::Set(uint64_t base, uint64_t step)
{
	if (IsSet()) {
		throw std::runtime_error("Sequence is already set");
	}
	
	IsSet_ = true;
	Base = base;
	Step = step;
}

uint64_t Sequence::GetNextValue()
{
	const uint64_t current_value = Base;
	const uint64_t diff = std::numeric_limits<uint64_t>::max() - Base;
	if (Step - 1 >= diff) {
		Base = Step - diff - 1;
	} else {
		Base += Step;
	}
	
	return current_value;
}
