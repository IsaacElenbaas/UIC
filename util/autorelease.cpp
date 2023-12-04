#include "user.h"
#include "util.h"

AutoRelease::AutoRelease(double time_ms, const InputTransformation& out_trns) : time_ms(time_ms), out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_AUTO_RELEASE, out_trns.hash, {});
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double));
	TRNS_DISCARD_USED();
	data += sizeof(bool);
	double* time = (double*)data; data += sizeof(double);
	*time += elapsed;
}
double AutoRelease::get(int i) const { return out_trns.get(i); }
double AutoRelease::set(int i, double v) {
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double));
	TRNS_DISCARD_USED();
	bool* last = (bool*)data; data += sizeof(bool);
	double* time = (double*)data; data += sizeof(double);
	double ret = 0;
	if(v != 0) {
		if(!(*last)) {
			*time = 0;
			ret = v;
			push_timer(hash, time_ms, false);
		}
		else if(*time < time_ms) ret = v;
	}
	else if(*last) pop_timer(hash, -1, false);
	*last = v != 0;
	out_trns.set(i, ret);
	return ret;
}
