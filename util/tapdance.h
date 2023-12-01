#ifndef TAPDANCE_H
#define TAPDANCE_H
#include <vector>
#include "user.h"

class TapDance : public InputTransformation {
	double timeout_ms;
	std::vector<InputTransformation*> out;
public:
	TapDance(double time_ms, std::vector<InputTransformation*> out);
	~TapDance();
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
