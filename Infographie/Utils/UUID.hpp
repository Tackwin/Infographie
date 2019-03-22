#pragma once
#include <random>

#include "Common.hpp"

class Uuid_t {
public:

	constexpr Uuid_t() noexcept : uuid(count++) {
		// check for overflow
		assert(count != 0ULL);
	}

	constexpr void nullify() noexcept {
		uuid = 0;
	}

	constexpr operator bool() const noexcept {
		return *this != Uuid_t{ 0 };
	}

	constexpr bool operator==(const Uuid_t& other) const noexcept {
		return uuid == other.uuid;
	}
	constexpr bool operator!=(const Uuid_t& other) const noexcept {
		return uuid != other.uuid;
	}
	constexpr bool operator<(const Uuid_t& other) const noexcept {
		return uuid < other.uuid;
	}

	constexpr explicit operator unsigned long long() const noexcept {
		return uuid;
	}

private:
	constexpr Uuid_t(unsigned long long n) noexcept : uuid(n) {}

	static unsigned long long count;

	unsigned long long uuid;
	friend std::hash<Uuid_t>;

	friend void print(const Uuid_t&) noexcept;
public:
	static constexpr Uuid_t zero() noexcept {
		return { 0 };
	}

};

namespace std {
	template<>
	struct hash<Uuid_t> {
		constexpr size_t operator()(const Uuid_t& id) const {
			return (size_t)id.uuid;
		}
	};
};

extern void print(const Uuid_t& id) noexcept;
