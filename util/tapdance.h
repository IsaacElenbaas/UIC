#ifndef TAPDANCE_H
#define TAPDANCE_H
#include <vector>
#include "user.h"

// TapDances can be used to map multiple outputs to one input by tapping it repeatedly
// After timeout_ms elapses without the input firing again, the queued output will fire for one cycle (unless the input is still held)
// Extends are most likely necessary with the use of a TapDance
class TapDance : public InputTransformation {
	double timeout_ms;
	std::vector<InputTransformation*> out;
public:
	TapDance(double timeout_ms, std::vector<InputTransformation*> out);
	~TapDance();
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
