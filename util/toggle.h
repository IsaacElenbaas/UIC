#ifndef TOGGLE_H
#define TOGGLE_H
#include "user.h"

class Toggle : public InputTransformation {
	InputTransformation& out_trns;
public:
	Toggle(const InputTransformation& out_trns);
	~Toggle() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
