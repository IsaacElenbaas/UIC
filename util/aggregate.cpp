#include "user.h"
#include "util.h"

Aggregate::Aggregate(const InputTransformation& out_trns) : out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_AGGREGATE, out_trns.hash, {});
	uintptr_t data = trns_data(hash, sizeof(input::values));
	TRNS_DISCARD_USED();
}
double Aggregate::get(int i) const { return out_trns.get(i); }
double Aggregate::set(int i, double v) {
	uintptr_t data = trns_data(hash, sizeof(input::values));
	bool* used = (bool*)data; data += sizeof(bool);
	double* values = (double*)data; data += sizeof(double);
	if(!(*used)) {
		for(int j = 0; j < (int)(sizeof(input::values)/sizeof(((input*)0)->values[0])); j++) {
			values[j] = 0;
		}
		*used = true;
	}
	values[i] += v;
	return out_trns.set(i, values[i]);
}
