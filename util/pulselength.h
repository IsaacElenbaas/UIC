#ifndef PULSELENGTH_H
#define PULSELENGTH_H
#include "user.h"

// PulseLengths can be used to mimic an analog input with a digital one by pulsing it at different speeds
// They are similar to Steam Controller Configuration's Analog Emulation
// Steam allows its equivalent of min_state_ms to be between 5 and 500, and its default is 28
// min_state_ms should be at least the length of one physics frame (generally 1000/fps) - so Steam's default is OK (not at all ideal) for 35fps
// PulseLengths may still be helpful for games with analog "support," but which just treat them as digital
// In these cases a PulseLength, a Bound, and two Aggregates can map the input to itself
class PulseLength : public InputTransformation {
	double min_state_ms;
	bool neg = false;
	InputTransformation &out_trns_neg, &out_trns;
public:
	PulseLength(double max_events, const InputTransformation& out_trns);
	PulseLength(double max_events, const InputTransformation& out_trns_neg, const InputTransformation& out_trns);
	~PulseLength() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
