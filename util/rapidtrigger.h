#ifndef RAPIDTRIGGER_H
#define RAPIDTRIGGER_H
#include "user.h"

// RapidTriggers can be used to make analog sticks mapped to buttons more comfortable
// They will make any press of press_width press their output, but not reset until their input is released by at least release_width
class RapidTrigger : public InputTransformation {
	double press_width, release_width;
	InputTransformation& out_trns;
public:
	RapidTrigger(double press_width, double release_width, const InputTransformation& out_trns);
	~RapidTrigger() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
