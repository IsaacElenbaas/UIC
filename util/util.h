#ifndef UTIL_H
#define UTIL_H
#include <functional>
#include <vector>
#include "autorelease.h"

enum {
	TRNS_AUTORELEASE,
	TRNS_TAPDANCE,
	TRNS_MAX
};
uintptr_t trns_hash(int trns, const InputTransformation& base, const std::vector<InputTransformation*>& extras);
uintptr_t trns_data(uintptr_t hash, size_t size);

void push_timeout(uintptr_t hash, double timeout_ms, const std::function<void()>& func);
void pop_timeout(uintptr_t hash);

void TapDance(const InputTransformation& in, double timeout_ms, std::vector<InputTransformation*> out);
#endif
