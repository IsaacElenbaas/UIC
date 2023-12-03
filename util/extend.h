#ifndef EXTEND_H
#define EXTEND_H
#include "user.h"

// AutoReleases can be used to prevent an input from being released for a specified amount of time
// This time can be changed dynamically
// Firing an Extend again will reset the countdown, but note that it cannot be held at the time because of an Aggregate - 0ms AutoReleases could remove that limitation
// Combining Extends with AutoReleases may be useful
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
