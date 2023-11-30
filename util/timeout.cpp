#include <functional>
#include <list>
#include <tuple>
#include "user.h"

static std::forward_list<std::tuple<uintptr_t, double, std::function<void()>>> timeouts;

void push_timeout(uintptr_t hash, double timeout_ms, const std::function<void()>& func) {
	timeouts.push_front({hash, timeout_ms, func});
}

void pop_timeout(uintptr_t hash) {
	for(auto i = timeouts.begin(), last = timeouts.before_begin(); i != timeouts.end(); last = i++) {
		if(std::get<0>(*i) == hash) {
			timeouts.erase_after(last);
			break;
		}
	}
}

void trigger_timeouts() {
	for(auto i = timeouts.begin(), last = timeouts.before_begin(); i != timeouts.end(); last = i++) {
		std::get<1>(*i) -= elapsed;
		if(std::get<1>(*i) <= 0) {
			std::get<2>(*i)();
			timeouts.erase_after(i = last);
		}
	}
}
