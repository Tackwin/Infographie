#include "UUID.hpp"
#include <cstdio>

unsigned long long Uuid_t::count = 0ULL;

void print(const Uuid_t& id) noexcept {
	printf("%llu", id.uuid);
}

