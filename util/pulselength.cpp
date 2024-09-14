#include <algorithm>
#include <cmath>
#include "user.h"
#include "util.h"

PulseLength::PulseLength(double min_state_ms, const InputTransformation& out_trns) : min_state_ms(min_state_ms), out_trns_neg(const_cast<InputTransformation&>(out_trns)), out_trns(const_cast<InputTransformation&>(out_trns)) {
	if(min_state_ms < 0)
		throw std::runtime_error("PulseLength min_state_ms was not greater than 0!");
	_hash = trns_hash(TRNS_PULSE_LENGTH, out_trns.hash, {});
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double)+sizeof(double));
	TRNS_DISCARD_USED();
	data += sizeof(bool);
	double* time = (double*)data; data += sizeof(double);
	*time += elapsed;
}
PulseLength::PulseLength(double min_state_ms, const InputTransformation& out_trns_neg, const InputTransformation& out_trns) : min_state_ms(min_state_ms), neg(true), out_trns_neg(const_cast<InputTransformation&>(out_trns_neg)), out_trns(const_cast<InputTransformation&>(out_trns)) {
	if(min_state_ms < 0)
		throw std::runtime_error("PulseLength min_state_ms was not greater than 0!");
	_hash = trns_hash(TRNS_PULSE_LENGTH, out_trns.hash, {const_cast<InputTransformation*>(&out_trns_neg)});
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double)+sizeof(double));
	TRNS_DISCARD_USED();
	data += sizeof(bool);
	double* time = (double*)data; data += sizeof(double);
	*time += elapsed;
}
double PulseLength::get(int i) const { return out_trns.get(i); }
double PulseLength::set(int i, double v) {
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double)+sizeof(double));
	TRNS_DISCARD_USED();
	bool* last = (bool*)data; data += sizeof(bool);
	double* time = (double*)data; data += sizeof(double);
	double* value = (double*)data; data += sizeof(double);
	if(v != 0) {
		double low = min_state_ms;
		double high = 2*fabs(v)*(1-fabs(v));
		if(high != 0) high = low/high;
		if(fabs(v) < 0.5) std::swap(low, high);
		if(!(*last) || *time >= ((*value == 0) ? low : high)) {
			*time = 0;
			if(*value == 0 || high == 0) {
				*value = 1;
				if(high != 0) push_timer(hash, high, false);
			}
			else {
				*value = 0;
				if(low != 0) push_timer(hash, low, false);
			}
		}
		else {
			// because we check low and high based on their *current* values for early adjustment, timers can trigger too early - so always refresh them
			pop_timer(hash, -1, false);
			push_timer(hash, (*value == 0) ? low : high, false);
		}
	}
	else {
		if(*last) pop_timer(hash, -1, false);
		*value = 0;
	}
	*last = v != 0;
	if(neg) ((v < 0) ? out_trns : out_trns_neg).set(i, 0);
	return ((neg && v < 0) ? out_trns_neg : out_trns).set(i, *value);
}
