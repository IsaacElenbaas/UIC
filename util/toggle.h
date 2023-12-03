#ifndef TOGGLE_H
#define TOGGLE_H
#include "user.h"

// Toggles can be used to turn inputs into switches
// Firing a Toggle once will turn the output on, and firing it again will turn the output off
class Toggle : public InputTransformation {
	InputTransformation& out_trns;
public:
	Toggle(const InputTransformation& out_trns);
	~Toggle() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
