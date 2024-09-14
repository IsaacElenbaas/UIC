#include <cmath>
#include "user.h"
#include "util.h"

RapidTrigger::RapidTrigger(double press_width, double release_width, const InputTransformation& out_trns) : press_width(press_width), release_width(release_width), out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_RAPID_TRIGGER, out_trns.hash, {});
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double));
	TRNS_CHECK_USED();
}
double RapidTrigger::get(int i) const { return out_trns.get(i); }
double RapidTrigger::set(int i, double v) {
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double));
	TRNS_DISCARD_USED();
	bool* pressed = (bool*)data; data += sizeof(bool);
	double* from = (double*)data; data += sizeof(double);
	double ret = (*pressed) ? 1 : 0;
	if(!(*pressed)) {
		*from = std::min(*from, v);
		if(v >= *from+press_width) {
			ret = 1;
			*from = v;
			*pressed = true;
		}
	}
	else {
		*from = std::max(*from, v);
		if(v <= *from-release_width) {
			ret = 0;
			*from = v;
			*pressed = false;
		}
	}
	return out_trns.set(i, ret);
}
