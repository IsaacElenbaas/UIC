#ifndef BOUNDS_H
#define BOUNDS_H
#include <tuple>
#include <vector>
#include "user.h"

// Bounds can be used to change the mapping of a, usually analog, input
// Some examples are inverting them or making a peak at 0.5 with zeroes at 0 and 1
// The sensitivity, or ramp-up speed, over a range can also be modified (see below)
class Bounds : public InputTransformation {
	std::vector<std::tuple<double, double, double, double, double>> list;
	InputTransformation& out_trns;
public:
	// in low, in high, out low, out high, sensitivity
	// sensitivity is between 0 and 1, 0.5 is base
	// beyond 0.25 or 0.75 will remove resolution
	Bounds(std::vector<std::tuple<double, double, double, double, double>> list, const InputTransformation& out_trns);
	Bounds(std::vector<std::tuple<double, double, double, double>> list, const InputTransformation& out_trns);
	~Bounds() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
