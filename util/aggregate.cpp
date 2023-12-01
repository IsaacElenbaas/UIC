#include "user.h"
#include "util.h"

Aggregate::Aggregate(const InputTransformation& out_trns) : out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_AGGREGATE, out_trns.hash, {});
	uintptr_t data = trns_data(hash, sizeof(double));
	TRNS_DISCARD_USED();
}
double Aggregate::get(int i) const { return out_trns.get(i); }
double Aggregate::set(int i, double v) {
	uintptr_t data = trns_data(hash, sizeof(double));
	bool* used = (bool*)data; data += sizeof(bool);
	double* value = (double*)data; data += sizeof(double);
	if(!(*used)) {
		*value = v;
		*used = true;
	}
	else *value += v;
	out_trns.set(i, *value);
	return *value;
}
