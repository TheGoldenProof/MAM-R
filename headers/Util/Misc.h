#pragma once
#include "TGLib\TGLib.h"
#include <functional>

#pragma region Hashing

template<typename Iter>
size_t hash_range(Iter begin, Iter end) {
	size_t seed = 0;
	for (Iter i = begin; i != end; i++) {
		seed ^= std::hash<typename std::iterator_traits<Iter>::value_type>{}(*i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	return seed;
}

template<typename T, typename... Rest>
void hash_combine(usize& seed, const T& v, const Rest&... rest) {
	seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	(hash_combine(seed, rest), ...);
}

#define MAKE_HASHABLE(type, ...) \
	namespace std { \
		template<> struct hash<type> { \
			size_t operator()(const type& t) const { \
				size_t ret = 0; \
				hash_combine(ret, __VA_ARGS__); \
				return ret; \
			} \
		}; \
	}
#pragma endregion