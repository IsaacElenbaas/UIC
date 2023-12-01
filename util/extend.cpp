#include "user.h"
#include "util.h"

Extend::Extend(double time_ms, const InputTransformation& out_trns) : time_ms(time_ms), out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_EXTEND, time_ms, {const_cast<InputTransformation*>(&out_trns)});
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
	bool pressed = v != 0;
	double ret = 0;
	if(pressed) {
		if(!(*last)) {
			*time = 0;
			push_timer(hash, time_ms, false);
		}
		out_trns.set(i, ret = *value = v);
	}
	else if(*value != 0) {
		if(*time >= time_ms) {
			out_trns.set(i, *value = 0);
		}
		else out_trns.set(i, ret = *value);
	}
	else out_trns.set(i, 0);
	*last = pressed;
	return ret;
}