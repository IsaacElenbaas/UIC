#include <functional>
#include <vector>
#include "user.h"
#include "util.h"

void TapDance(const InputTransformation& in, double timeout_ms, std::vector<InputTransformation*> out) {
	uintptr_t hash = trns_hash(TRNS_TAPDANCE, in, out);
	uintptr_t data = trns_data(hash, sizeof(bool)+sizeof(int)+sizeof(double));
	bool* last = (bool*)data; data += sizeof(bool);
	int* stage = (int*)data; data += sizeof(int);
	double* time = (double*)data; data += sizeof(double);
	bool pressed = in.get(0) == 1;
	if((!(*last)) && pressed) {
		(*stage)++;
		*time = 0;
		pop_timer(hash, -1, false);
		push_timer(hash, timeout_ms, false);
	}
	else if(*stage != 0) *time += elapsed;
	*last = pressed;
	if(*stage == (int)out.size() || *time >= timeout_ms) {
		out[*stage-1]->set(0, 1);
		*stage = 0;
		*time = 0;
		pop_timer(hash, -1, false);
	}
	for(auto i = out.begin(); i != out.end(); ++i) { delete *i; }
}
