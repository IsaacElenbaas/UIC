#include <functional>
#include "user.h"
#include "util.h"

TapDance::TapDance(double timeout_ms, std::vector<InputTransformation*> out) : timeout_ms(timeout_ms), out(out) {
	_hash = trns_hash(TRNS_TAP_DANCE, timeout_ms, out);
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(int)+sizeof(double));
	TRNS_CHECK_USED();
	data += sizeof(bool);
	data += sizeof(int);
	double* time = (double*)data; data += sizeof(double);
	*time += elapsed;
}
TapDance::~TapDance() {
	for(auto i = out.begin(); i != out.end(); ++i) { delete *i; }
}
double TapDance::get(int i) const {
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(int)+sizeof(double));
	TRNS_CHECK_USED();
	data += sizeof(bool);
	int* stage = (int*)data; data += sizeof(int);
	return (*stage == 0) ? 0 : out[*stage-1]->get(i);
}
double TapDance::set(int i, double v) {
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(int)+sizeof(double));
	TRNS_DISCARD_USED();
	bool* last = (bool*)data; data += sizeof(bool);
	int* stage = (int*)data; data += sizeof(int);
	double* time = (double*)data; data += sizeof(double);
	double ret = 0;
	if((!(*last)) && v != 0) {
		(*stage)++;
		*time = 0;
		pop_timer(hash, -1, false);
		push_timer(hash, timeout_ms, false);
	}
	for(int j = 0; j < (int)out.size(); j++) {
		if(j != *stage-1) out[j]->set(i, 0);
	}
	if(*stage == (int)out.size() || (*stage != 0 && *time >= timeout_ms)) {
		out[*stage-1]->set(i, ret = 1);
		*stage = 0;
		*time = 0;
		pop_timer(hash, -1, false);
	}
	else if(*stage != 0) out[*stage-1]->set(i, 0);
	*last = v != 0;
	return ret;
}
