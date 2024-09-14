#include <cmath>
#include "user.h"
#include "util.h"

Thresholds::Thresholds(double release, double press, const InputTransformation& out_trns) : release(release), press(press), out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_THRESHOLDS, out_trns.hash, {});
	uintptr_t data = trns_data(hash, sizeof(bool));
	TRNS_CHECK_USED();
}
double Thresholds::get(int i) const { return out_trns.get(i); }
double Thresholds::set(int i, double v) {
	uintptr_t data = trns_data(hash, sizeof(bool));
	TRNS_DISCARD_USED();
	bool* pressed = (bool*)data; data += sizeof(bool);
	double ret = (*pressed) ? 1 : 0;
	if(!(*pressed)) {
		if(v >= press) {
			ret = 1;
			*pressed = true;
		}
	}
	else {
		if(v <= release) {
			ret = 0;
			*pressed = false;
		}
	}
	return out_trns.set(i, ret);
}
