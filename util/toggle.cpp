#include "user.h"
#include "util.h"

Toggle::Toggle(const InputTransformation& out_trns) : out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_TOGGLE, out_trns.hash, {});
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double));
	TRNS_DISCARD_USED();
}
double Toggle::get(int i) const { return out_trns.get(i); }
double Toggle::set(int i, double v) {
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(double));
	TRNS_DISCARD_USED();
	bool* last = (bool*)data; data += sizeof(bool);
	double* value = (double*)data; data += sizeof(double);
	if(v != 0) {
		if(!(*last))
			*value = (*value == 0) ? v : 0;
	}
	*last = v != 0;
	return out_trns.set(i, *value);
}
