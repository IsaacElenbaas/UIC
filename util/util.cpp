#include <bit>
#include <climits>
#include <cmath>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "user.h"
#include "util.h"

uintptr_t trns_hash(int trns, const InputTransformation& base, const std::vector<InputTransformation*>& extras) {
	uintptr_t hash = std::hash<uintptr_t>()(base.hash);
	for(auto i = extras.begin(); i != extras.end(); ++i) {
		hash = (hash+((CHAR_BIT*sizeof(uintptr_t) < 64) ? 0x9E3779B9 : 0x9E3779B97F4A7C15)) ^ std::hash<uintptr_t>()((*i)->hash) >> (CHAR_BIT*sizeof(uintptr_t)/2);
	}
	return (hash << std::bit_width((unsigned int)TRNS_MAX)) | trns;
}

static std::unordered_map<uintptr_t, std::vector<uint8_t>> trns_data_storage;
uintptr_t trns_data(uintptr_t hash, size_t size) {
	auto data = trns_data_storage.find(hash);
	size_t vec_size = std::ceil(size*(((double)CHAR_BIT)/8));
	if(data != trns_data_storage.end()) {
		if(std::get<1>(*data).size() != vec_size)
			throw std::runtime_error("InputTransformation hash collision!");
		return (uintptr_t)std::get<1>(*data).data();
	}
	trns_data_storage.insert(std::pair<uintptr_t, std::vector<uint8_t>>(hash, std::vector<uint8_t>()));
	trns_data_storage[hash].resize(vec_size);
	std::memset(trns_data_storage[hash].data(), 0, size);
	return (uintptr_t)trns_data_storage[hash].data();
}
