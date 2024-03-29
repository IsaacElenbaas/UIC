#ifndef UTIL_H
#define UTIL_H
#include <functional>
#include <stdexcept>
#include <vector>
#include "aggregate.h"
#include "autorelease.h"
#include "bounds.h"
#include "deadzone.h"
#include "extend.h"
#include "noop.h"
#include "pulselength.h"
#include "rapidtrigger.h"
#include "tapdance.h"
#include "thresholds.h"
#include "toggle.h"

#define TRNS_CHECK_USED() bool* used = (bool*)data; data += sizeof(bool); if(*used) throw std::runtime_error("InputTransformation hash collision!"); else *used = true;
#define TRNS_DISCARD_USED() data += sizeof(bool);

enum {
	TRNS_AGGREGATE,
	TRNS_AUTO_RELEASE,
	TRNS_BOUNDS,
	TRNS_DEADZONE,
	TRNS_EXTEND,
	TRNS_NOOP,
	TRNS_PULSE_LENGTH,
	TRNS_RAPID_TRIGGER,
	TRNS_TAP_DANCE,
	TRNS_THRESHOLDS,
	TRNS_TOGGLE,
	TRNS_MAX
};
uintptr_t trns_hash(int trns, uintptr_t base, const std::vector<InputTransformation*> extras);
uintptr_t trns_data(uintptr_t hash, size_t size);

void push_timeout(uintptr_t hash, double timeout_ms, const std::function<void()>& func);
void pop_timeout(uintptr_t hash);
#endif
