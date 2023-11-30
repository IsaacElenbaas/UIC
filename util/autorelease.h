#ifndef AUTORELEASE_H
#define AUTORELEASE_H
#include "user.h"

class AutoRelease : public InputTransformation {
	double time_ms;
	InputTransformation& out_trns;
public:
	AutoRelease(double time_ms, const InputTransformation& out_trns);
	~AutoRelease() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
