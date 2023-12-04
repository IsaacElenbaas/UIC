#ifndef THRESHOLDS_H
#define THRESHOLDS_H
#include "user.h"

// Thresholds can be used to make analog sticks mapped to buttons
// They will make any press beyond press press their output, but not reset until their input is released to or past release
// Using multiple on one trigger can allow different actions for soft / full pull
class Thresholds : public InputTransformation {
	double release, press;
	InputTransformation& out_trns;
public:
	Thresholds(double release, double press, const InputTransformation& out_trns);
	~Thresholds() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
