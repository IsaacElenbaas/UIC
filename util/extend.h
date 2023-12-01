#ifndef EXTEND_H
#define EXTEND_H
#include "user.h"

class Extend : public InputTransformation {
	double time_ms;
	InputTransformation& out_trns;
public:
	Extend(double time_ms, const InputTransformation& out_trns);
	~Extend() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
