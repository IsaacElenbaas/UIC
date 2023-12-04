#include <algorithm>
#include <cmath>
#include "user.h"
#include "util.h"

Bounds::Bounds(std::vector<std::tuple<double, double, double, double, double>> list, const InputTransformation& out_trns) : list(list), out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = trns_hash(TRNS_BOUNDS, out_trns.hash, {});
}
Bounds::Bounds(std::vector<std::tuple<double, double, double, double>> list, const InputTransformation& out_trns) : out_trns(const_cast<InputTransformation&>(out_trns)) {
	for(auto i = list.begin(); i != list.end(); ++i) {
		this->list.push_back(std::tuple_cat(*i, std::make_tuple(0.5)));
	}
}
double Bounds::get(int i) const { return out_trns.get(i); }
static inline bool in_range(double a, double b, double c) {
	return std::min(a, b) <= c && c <= std::max(a, b);
}
double Bounds::set(int i, double v) {
	for(auto i = list.begin(); i != list.end(); ++i) {
		if(in_range(std::get<0>(*i), std::get<1>(*i), v)) {
			if(std::get<2>(*i) == std::get<3>(*i)) { v = std::get<2>(*i); break; }
			if(std::get<4>(*i) == 0) { v = (v == 0) ? std::get<2>(*i) : std::get<3>(*i); break; }
			if(std::get<4>(*i) == 1) { v = (v == 1) ? std::get<3>(*i) : std::get<2>(*i); break; }
			v = (v-std::get<0>(*i))/(std::get<1>(*i)-std::get<0>(*i));
			if(std::get<4>(*i) != 0.5) {
				double sens = 2*(std::get<4>(*i)-0.5);
				double m = std::max(0.0, 1-2*fabs(sens));
				double h = std::min(1.0, 2-2*fabs(sens));
				double a = (m > 0) ? 1-m : 1/pow(h, 3);
				double b = -3*a;
				if(sens < 0) v = 1-v;
				v = std::clamp(a*pow(v+1-h, 3.0)+b*pow(v+1-h, 2.0)+h*(3*a+2*b-m)+v*(m-3*a-2*b)-a-b+1, 0.0, 1.0);
				if(sens < 0) v = 1-v;
			}
			else v = std::clamp(v, 0.0, 1.0);
			v = v*(std::get<3>(*i)-std::get<2>(*i))+std::get<2>(*i);
			break;
		}
	}
	out_trns.set(i, v);
	return v;
}
