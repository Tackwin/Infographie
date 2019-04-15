#pragma once
#include <set>
#include <bitset>
#include <cassert>
#include <functional>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <any>

#include <GL/glew.h>
#include <GL/GL.h>

namespace details {
	struct Defer {
	private:
		std::function<void(void)> todo;

	public:
		Defer() = default;
		~Defer() {
			if (todo) todo();
		}

		template<typename Callable>
		Defer(Callable&& todo) noexcept {
			this->todo = todo;
		};
	};
};

#define defer details::Defer _CONCAT(defer_, __COUNTER__) = [&]
#define BEG_END(x) std::begin(x), std::end(x)
namespace xstd {
	constexpr size_t constexpr_pow2(size_t D) noexcept {
		size_t p = 1;
		for (size_t i = 0; i < D; ++i, p *= 2);
		return p;
	}

	template<typename T>
	constexpr int sign(T x) noexcept {
		return (T(0) < x) - (x < T(0));
	}

	template<typename T, typename U>
	std::enable_if_t<
		std::is_convertible_v<T, std::string> && std::is_constructible_v<U, std::string>,
		std::string
	> append(T a, U b) {
		return std::string{ a } +b;
	}

	template<typename T>
	constexpr inline std::size_t hash_combine(std::size_t seed, const T& v) noexcept {
		std::hash<T> h;
		seed ^= h(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
	}
	constexpr inline std::size_t hash_combine(std::size_t a, std::size_t b) noexcept {
#pragma warning(push)
#pragma warning(disable: 4307)
		return a + 0x9e3779b9u + (b << 6) + (b >> 2);
#pragma warning(pop)
	}

	template<size_t S>
	constexpr std::bitset<S> full_bitset() noexcept {
		std::bitset<S> bitset;
		for (size_t i = 0; i < S; ++i) bitset.set(i, true);
		return bitset;
	}

	template<size_t S>
	constexpr std::bitset<S> consecutive_to_bitset() noexcept {
		return std::bitset<S>{};
	}

	template<size_t S, typename... Args>
	constexpr std::bitset<S> consecutive_to_bitset(size_t x, Args... args) noexcept {
		std::bitset<S> bs;
		bs.set(x, true);
		if constexpr (sizeof...(Args) == 0) {
			return bs;
		}

		return bs | consecutive_to_bitset<S>(args...);
	}
};

namespace std {
	template<typename T>
	struct hash<std::pair<T, T>> {
		size_t operator()(const std::pair<size_t, size_t>& x) const noexcept {
			return xstd::hash_combine(std::hash<T>()(x.first), x.second);
		}
	};

	template<>
	struct hash<std::set<size_t>> {
		size_t operator()(const std::set<size_t>& x) const noexcept {
			size_t seed = 0;
			for (const auto& v : x) {
				seed = xstd::hash_combine(seed, v);
			}
			return seed;
		}
	};
	template<>
	struct hash<std::unordered_set<size_t>> {
		size_t operator()(const std::unordered_set<size_t>& x) const noexcept {
			size_t seed = 0;
			for (const auto& v : x) {
				seed = xstd::hash_combine(seed, v);
			}
			return seed;
		}
	};
};

class Assets_Manager;
struct Matrix4f;
namespace Common {
	extern Assets_Manager* AM;
	extern std::filesystem::path Base_Working_Directory;
	constexpr size_t SEED = 0;
	constexpr float PIf = 3.1415296f;
	constexpr double PI = 3.1415926535898;
	constexpr double RAD_2_DEG = 57.295779513;

#pragma warning(push)
#pragma warning(disable: 4307)
	constexpr size_t operator""_id(const char* user, size_t size) {
		size_t seed = 0;
		for (size_t i = 0; i < size; ++i) seed = xstd::hash_combine(seed, (size_t)user[i]);
		return seed;
	}
	extern void GLAPIENTRY verbose_opengl_error(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const char* message,
		GLvoid* userParam
	) noexcept;

	extern std::unordered_map<std::string, std::any> debug_values;

	extern bool Is_In_Sfml_Context;
	extern Matrix4f* View_Matrix;
	extern Matrix4f* Projection_Matrix;

	extern float Alpha_Tolerance;
}
using namespace Common;

