#include <cmath>
#include "user.h"
#include "util.h"

Deadzone::Deadzone(double deadzone, const InputTransformation& out_trns) : deadzone(deadzone), out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_DEADZONE, out_trns.hash, {});
}
double Deadzone::get(int i) const { return out_trns.get(i); }
double Deadzone::set(int i, double v) {
	if(deadzone >= 0) {
		if(fabs(v) <= deadzone) v = 0;
		else v = (v-std::copysign(deadzone, v))/(1-deadzone);
	}
	else {
		if(fabs(v) >= 1+deadzone) v = std::copysign(1, v);
		else v = v/(1+deadzone);
	}
	return out_trns.set(i, v);
}
