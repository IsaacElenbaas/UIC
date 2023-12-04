#include <climits>
#include "user.h"
#include "util.h"

Noop::Noop() {
	_hash = trns_hash(TRNS_NOOP, 0, {});
}
Noop::Noop(const InputTransformation& out_trns) {
	_hash = trns_hash(TRNS_NOOP, out_trns.hash, {});
}
double Noop::get(int i) const { (void)i; return 0; }
double Noop::set(int i, double v) { (void)i; return v; }

Rot::Rot(const InputTransformation& out_trns) : out_trns(const_cast<InputTransformation&>(out_trns)) {
	_hash = (out_trns.hash << 1) & (out_trns.hash >> (sizeof(out_trns.hash)*CHAR_BIT-1));
}
double Rot::get(int i) const { return out_trns.get(i); }
double Rot::set(int i, double v) { return out_trns.set(i, v); }
