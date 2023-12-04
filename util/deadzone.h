#ifndef DEADZONE_H
#define DEADZONE_H
#include "user.h"

// Deadzones are self-explanatory
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
