#include "UUID.hpp"
#include <cstdio>

unsigned long long UUID::count = 0ULL;

void print(const UUID& id) noexcept {
	printf("%llu", id.uuid);
}

