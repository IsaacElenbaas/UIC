#ifndef DEADZONE_H
#define DEADZONE_H
#include "user.h"

// Deadzones are self-explanatory
// A negative deadzone will apply it to the outer edge instead of the center
class Deadzone : public InputTransformation {
	double deadzone;
	InputTransformation& out_trns;
public:
	Deadzone(double deadzone, const InputTransformation& out_trns);
	~Deadzone() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
