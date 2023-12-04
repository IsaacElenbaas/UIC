#include "user.h"
#include "util.h"

// Extends extend the length of a press
Extend::Extend(double time_ms, const InputTransformation& out_trns) : time_ms(time_ms), out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_EXTEND, out_trns.hash, {});
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double)+sizeof(double));
	TRNS_DISCARD_USED();
	data += sizeof(bool);
	double* time = (double*)data; data += sizeof(double);
	*time += elapsed;
}
double Extend::get(int i) const { return out_trns.get(i); }
double Extend::set(int i, double v) {
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double)+sizeof(double));
	TRNS_DISCARD_USED();
	bool* last = (bool*)data; data += sizeof(bool);
	double* time = (double*)data; data += sizeof(double);
	double* value = (double*)data; data += sizeof(double);
	if(v != 0) {
		if(!(*last)) {
			*time = 0;
			push_timer(hash, time_ms, false);
		}
		*value = v;
	}
	else if(*time >= time_ms)
		*value = 0;
	*last = v != 0;
	out_trns.set(i, *value);
	return *value;
}
