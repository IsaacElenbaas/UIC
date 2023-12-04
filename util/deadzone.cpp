#include <cmath>
#include "user.h"
#include "util.h"

Deadzone::Deadzone(double deadzone, const InputTransformation& out_trns) : deadzone(deadzone), out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_DEADZONE, out_trns.hash, {});
}
double Deadzone::get(int i) const { return out_trns.get(i); }
double Deadzone::set(int i, double v) {
	if(fabs(v) < deadzone) v = 0;
	else v = (v-std::copysign(deadzone, v))/(1-deadzone);
	out_trns.set(i, v);
	return v;
}
