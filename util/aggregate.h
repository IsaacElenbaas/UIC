#ifndef AGGREGATE_H
#define AGGREGATE_H
#include "user.h"

class Aggregate : public InputTransformation {
	InputTransformation& out_trns;
public:
	Aggregate(const InputTransformation& out_trns);
	~Aggregate() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
