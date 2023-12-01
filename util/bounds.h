#ifndef BOUNDS_H
#define BOUNDS_H
#include <tuple>
#include <vector>
#include "user.h"

class Bounds : public InputTransformation {
	std::vector<std::tuple<double, double, double, double, double>> list;
	InputTransformation& out_trns;
public:
	// in low, in high, out low, out high, sensitivity
	// sensitivity is between 0 and 1, 0.5 is base
	// beyond 0.25 or 0.75 will remove resolution
	Bounds(std::vector<std::tuple<double, double, double, double, double>> list, const InputTransformation& out_trns);
	~Bounds() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
