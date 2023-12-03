#ifndef AGGREGATE_H
#define AGGREGATE_H
#include "user.h"

// Aggregates can be used to sum the values of each input given to them
// They are useful when multiple inputs are in some way mapped to one
// Without the use of Aggregates, only the last used would have any effect
class Aggregate : public InputTransformation {
	InputTransformation& out_trns;
public:
	Aggregate(const InputTransformation& out_trns);
	~Aggregate() {};
	double get(int i) const override;
	double set(int i, double v) override;
};
#endif
