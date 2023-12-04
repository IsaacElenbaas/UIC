#ifndef NOOP_H
#define NOOP_H
#include "user.h"

// Noops do not do anything, and will ignore any set calls
// They are useful because they still pass through a hash value
// A unique combination of hash values is necessary for some InputTransformations
// Rots pass through values, but rotate the hash value
// They are useful for e.g. using multiple Extends for each axis of a 2D input which would otherwise have issues
class Noop : public InputTransformation {
public:
	Noop();
	Noop(const InputTransformation& out_trns);
	~Noop() {};
	double get(int i) const override;
	double set(int i, double v) override;
};

class Rot : public InputTransformation {
	InputTransformation& out_trns;
public:
	Rot(const InputTransformation& out_trns);
	~Rot() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
