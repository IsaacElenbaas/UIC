#ifndef AUTORELEASE_H
#define AUTORELEASE_H
#include "user.h"

// AutoReleases can be used to automatically release an input after a specified amount of time
// This time can be changed dynamically
// Combining AutoReleases with Extends may be useful
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
