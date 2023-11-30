#include "user.h"
#include "util.h"

AutoRelease::AutoRelease(double time_ms, const InputTransformation& out_trns) : time_ms(time_ms), out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_AUTORELEASE, out_trns, {});
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double));
	bool* last = (bool*)data; data += sizeof(bool);
	double* time = (double*)data; data += sizeof(double);
	*time += elapsed;
	if(*time >= time_ms) {
		if(*last) AutoRelease::out_trns.set(0, 0);
		*last = false;
	}
}
double AutoRelease::get(int i) const { (void)i; return out_trns.get(0); }
double AutoRelease::set(int i, double v) {
	(void)i;
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double));
	bool* last = (bool*)data; data += sizeof(bool);
	double* time = (double*)data; data += sizeof(double);
	bool pressed = v != 0;
	if((!(*last)) && pressed) {
		*time = 0;
		out_trns.set(0, v);
		push_timer(hash, time_ms, false);
	}
	else if((*last) && !pressed) {
		out_trns.set(0, 0);
		pop_timer(hash, -1, false);
	}
	*last = pressed;
	return v;
}
