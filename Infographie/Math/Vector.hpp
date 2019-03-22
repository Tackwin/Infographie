#pragma once
#include <random>
#include <optional>
#include <type_traits>

#include "Common.hpp"

#pragma warning(push)
#pragma warning(disable: 4201)

#define EXPAND(x) x

#define UNROLL_2(v) (v).x, (v).y
#define UNROLL_2_P(v, prefix) (prefix)((v).x), (prefix)((v).y)
#define UNROLL_3(v) (v).x, (v).y, (v).z
#define UNROLL_3_P(v, prefix) (prefix)(v).x, (prefix)(v).y (prefix)(v).z

#define COLOR_UNROLL(x) (x).r, (x).g, (x).b, (x).a
#define COLOR_UNROLL_P(x, prefix) \
	(prefix)((x).r), (prefix)((x).g), (prefix)((x).b), (prefix)((x).a)
#define XYZW_UNROLL(v) (v).x, (v).y, (v).z, (v).w

static constexpr auto Vector2_Type_Tag = "Vector2<T>"_id;

template<size_t D, typename T>
struct Vector;
template<typename T> using Vector2 = Vector<2U, T>;
template<typename T> using Vector3 = Vector<3U, T>;
template<typename T> using Vector4 = Vector<4U, T>;
using Vector2u = Vector2<size_t>;
using Vector2i = Vector2<int>;
using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector3f = Vector3<float>;
using Vector3u = Vector3<size_t>;
using Vector3i = Vector3<int>;
using Vector4u = Vector4<size_t>;
using Vector4f = Vector4<float>;
using Vector4d = Vector4<double>;
using Vector4b = Vector4<uint8_t>;

template<size_t D, typename T>
struct __vec_member {
	T components[D];
};
template<typename T>
struct __vec_member<1, T> {
	union {
		struct {
			T x;
		};
		T components[1];
	};

	__vec_member() : x(0) {}
	__vec_member(T x) : x(x) {}
};
template<typename T>
struct __vec_member<2, T> {
	union {
		struct {
			T x;
			T y;
		};
		T components[2];
	};

	__vec_member() : x(0), y(0) {}
	__vec_member(T x, T y) : x(x), y(y) {}
};
template<typename T>
struct __vec_member<3, T> {
	union {
		struct {
			T x;
			T y;
			T z;
		};
		struct {
			T h;
			T s;
			T l;
		};
		T components[3];
	};

	__vec_member() : x{}, y{}, z{} {}
	__vec_member(T x, T y, T z) : x(x), y(y), z(z) {}
};
template<typename T>
struct __vec_member<4, T> {
	union {
		struct {
			T x;
			T y;
			T z;
			T w;
		};
		struct {
			T h;
			T s;
			T l;
			T a;
		};
		struct {
			T r;
			T g;
			T b;
			T a;
		};
		Vector3f xyz;
		Vector3f rgb;
		T components[4];
	};

	__vec_member() : x(0), y(0), z(0), w(0) {}
	__vec_member(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
};

template<size_t D, typename T = float>
struct Vector : public __vec_member<D, T> {

#pragma region STATIC

	static Vector<D, T> createUnitVector(float angles[D]) {
		Vector<D, T> result;
		result[0] = cosf(angles[0]);
		for (size_t i = 1u; i < D; ++i) {

			result[i] = (i + 1u != D) ?
				cosf(angles[i]) :
				1;

			for (size_t j = 0u; j < D - 1u; ++j) {
				result[i] *= sinf(angles[j]);
			}
		}
		return result;
	}
	static Vector<2, T> createUnitVector(float angles) { // i'm not doing all the shit above for 2d
		return { cosf(angles), sinf(angles) };
	}
	static Vector<D, T> createUnitVector(double angles[D]) {
		Vector<D, T> result;
		result[0] = static_cast<T>(cos(angles[0]));
		for (size_t i = 1u; i < D; ++i) {

			result[i] = (i + 1u != D) ?
				static_cast<T>(cos(angles[i])) :
				1;

			for (size_t j = 0u; j < D - 1u; ++j) {
				result[i] *= static_cast<T>(sin(angles[j]));
			}
		}
		return result;
	}
	static Vector<2, T> createUnitVector(double angles) { // i'm not doing all the shit above for 2d
		return {
			static_cast<T>(cos(angles)),
			static_cast<T>(sin(angles))
		};
	}
	template<typename V>
	static bool equalf(const V& A, const V& B, float eps = FLT_EPSILON) {
		return (A - B).length2() <= eps * eps;
	}
	template<typename V>
	static bool equal(const V& A, const V& B, double eps = DBL_EPSILON) {
		return (A - B).length2() <= eps * eps;
	}
	static Vector<D, T> clamp(const Vector<D, T>& V, const Vector<D, T>& min, const Vector<D, T>& max) {
		Vector<D, T> result;
		for (size_t i = 0u; i < D; ++i) {
			result[i] = std::clamp(V[i], min[i], max[i]);
		}
		return result;
	}
	static Vector<D, T> rand(
		const Vector<D, T>& min,
		const Vector<D, T>& max,
		std::default_random_engine rng = std::default_random_engine()
	) {
		Vector<D, T> r;

		if constexpr (std::is_integral_v<T>) {
			for (size_t i = 0u; i < D; ++i) {
				r[i] = std::uniform_int_distribution<T>(min[i], max[i])(rng);
			}
			return r;
		}

		for (size_t i = 0u; i < D; ++i) {
			r[i] = std::uniform_real_distribution<T>(min[i], max[i])(rng);
		}

		return r;
	}
#pragma endregion

	constexpr Vector() {
		for (size_t i = 0u; i < D; ++i) {
			this->components[i] = T{};
		}
	}

	template<size_t Dp = D>
	constexpr Vector(T x, std::enable_if_t<Dp == 2, T> y) :
		__vec_member<2, T>(x, y)
	{}

	template<size_t Dp = D>
	constexpr Vector(T x, T y, std::enable_if_t<Dp == 3, T> z) :
		__vec_member<3, T>(x, y, z)
	{}

	template<size_t Dp = D>
	constexpr Vector(T x, T y, T z, std::enable_if_t<Dp == 4, T> w) :
		__vec_member<4, T>(x, y, z, w)
	{}

	size_t getDimension() const {
		return D;
	}

	Vector<D, T>& clamp(const Vector<D, T>& min, const Vector<D, T>& max) {
		for (size_t i = 0u; i < D; ++i) {
			this->components[i] = std::clamp(this->components[i], min[i], max[i]);
		}
		return *this;
	}

	template<typename U>
	bool inRect(const Vector<D, U>& pos, const Vector<D, U>& size) const {
		for (size_t i = 0u; i < D; ++i) {
			if (!(
				static_cast<T>(pos[i]) < this->components[i] &&
				this->components[i] < static_cast<T>(pos[i] + size[i])
				))
			{
				return false;
			}
		}

		return true;
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 3, Vector3<T>> cross(Vector3<T> other) const noexcept {
		return Vector3<T>{
			this->y * other.z - this->z * other.y,
			this->z * other.x - this->x * other.z,
			this->x * other.y - this->y * other.x
		};
	}

	T dot(const Vector<D, T>& other) const noexcept {
		T sum = 0;
		for (size_t i = 0; i < D; ++i) {
			sum += this->components[i] * other.components[i];
		}
		return sum;
	}

	T length() const {
		T result = 0;
		for (size_t i = 0u; i < D; ++i) {
			result += this->components[i] * this->components[i];
		}
		return sqrt(result);
	}

	T length2() const {
		T result = 0;
		for (size_t i = 0u; i < D; ++i) {
			result += this->components[i] * this->components[i];
		}
		return result;
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, double> angleX() const noexcept {
		return std::atan2(this->y, this->x);
	}

	template<typename U, size_t Dp = D>
	std::enable_if_t<Dp == 2, double> angleTo(const Vector<2U, U>& other) const noexcept {
		return std::atan2(other.y - this->y, other.x - this->x);
	}

	template<typename U, size_t Dp = D>
	std::enable_if_t<Dp == 2, double> angleFrom(const Vector<2U, U>& other) const noexcept {
		return std::atan2(this->y - other.y, this->x - other.x);
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, double> pseudoAngleX() const noexcept {
		auto dx = this->x;
		auto dy = this->y;
		return std::copysign(1.0 - dx / (std::fabs(dx) + fabs(dy)), dy);
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, double>
		pseudoAngleTo(const Vector<2U, T>& other) const noexcept {
		return (other - *this).pseudoAngleX();
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, double>
		pseudoAngleFrom(const Vector<2U, T>& other) const noexcept {
		return (*this - other).pseudoAngleX();
	}

	Vector<D, T>& normalize() {
		const auto& l = length();
		if (l == 0) return *this;
		for (size_t i = 0u; i < D; ++i) {
			this->components[i] /= l;
		}
		return *this;
	}

	Vector<D, T> round(T magnitude) {
		Vector<D, T> results;
		for (size_t i = 0; i < D; ++i) {
			results[i] = static_cast<T>(
				std::round(this->components[i] / magnitude) * magnitude
				);
		}
		return results;
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, Vector<D, T>> fitUpRatio(double ratio) const noexcept {
		if (this->x > this->y) {
			return { this->x, (T)(this->x / ratio) };
		}
		else {
			return { (T)(this->y * ratio), this->y };
		}
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, Vector<D, T>> fitDownRatio(double ratio) const noexcept {
		Vector<2, T> result;
		result.x = (T)ratio;
		result.y = (T)1;
		result *= std::max(this->x, this->y) / ratio;

		return result;
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, Vector<D, T>> fit_into_preserve_ratio(Vector<D, T> out) noexcept {
		auto scale = std::min(out.x / this->x, out.y / this->y);
		auto result = *this;
		result *= scale;
		return result;
	}

	template<typename L>
	std::enable_if_t<
		std::is_invocable_r_v<T, L, T>, Vector<D, T>
	> applyCW(L lamb) const noexcept {
		Vector<D, T> res;
		for (size_t i = 0; i < D; ++i) {
			res[i] = lamb(this->components[i]);
		}
		return res;
	}

#pragma region OPERATOR
	T& operator[](size_t i) {
		return this->components[i];
	}
	const T& operator[](size_t i) const {
		return this->components[i];
	}

	Vector<D, T> hamilton(Vector<D, T> other) noexcept {
		Vector<D, T> result = *this;
		for (size_t i = 0; i < D; ++i) result[i] *= other[i];
		return result;
	}

	template<typename U>
	Vector<D, T> operator*(const U& scalaire) const {
		static_assert(std::is_scalar<U>::value, "need to be a scalar");
		Vector<D, T> result;
		for (size_t i = 0; i < getDimension(); ++i) {
			result[i] = static_cast<T>(this->components[i] * scalaire);
		}

		return result;
	}
	template<typename U>
	Vector<D, T> operator/(const U& scalaire) const {
		static_assert(std::is_scalar<U>::value);
		Vector<D, T> result;
		for (size_t i = 0; i < getDimension(); ++i) {
			result[i] = static_cast<T>(this->components[i] / scalaire);
		}

		return result;
	}

	template<typename U>
	Vector<D, T> operator+(const U& other) const {
		Vector<D, T> result;
		for (size_t i = 0; i < getDimension(); ++i) {
			result[i] = static_cast<T>(this->components[i] + other[i]);
		}

		return result;
	}
	template<typename U>
	Vector<D, T> operator-(const U& other) const {
		Vector<D, T> result;
		for (size_t i = 0; i < getDimension(); ++i) {
			result[i] = static_cast<T>(this->components[i] - other[i]);
		}

		return result;
	}


	template<typename U>
	Vector<D, T>& operator+=(const U& other) {
		for (size_t i = 0; i < getDimension(); ++i) {
			this->components[i] += static_cast<T>(other[i]);
		}
		return *this;
	}
	template<typename U>
	Vector<D, T>& operator-=(const U& other) {
		return this->operator+=(static_cast<T>(-1) * other);
	}
	template<typename U>
	Vector<D, T>& operator*=(const U& scalaire) {
		static_assert(std::is_scalar<U>::value);
		for (size_t i = 0; i < getDimension(); ++i) {
			this->components[i] *= static_cast<T>(scalaire);
		}
		return *this;
	}
	template<typename U>
	Vector<D, T>& operator/=(const U& scalaire) {
		static_assert(std::is_scalar<U>::value);
		for (size_t i = 0; i < getDimension(); ++i) {
			this->components[i] /= static_cast<T>(scalaire);
		}
		return *this;
	}

	template<typename U>
	bool operator==(const Vector<D, U>& other) const {
		for (size_t i = 0u; i < D; ++i) {
			if (this->components[i] != other.components[i])
				return false;
		}
		return true;
	}
	template<typename U>
	bool operator!=(const Vector<D, U>& other) const {
		return !(this->operator==(other));
	}

	// this got to be the most convulated method signature that i've written.
	template<typename F>
	std::enable_if_t<
		std::is_invocable_r_v<T, F, T>,
		Vector<D, T>
	> apply(F&& f) noexcept(std::is_nothrow_invocable_v<F, T>) {
		Vector<D, T> result;
		for (auto i = 0; i < D; ++i) {
			result[i] = f(this->components[i]);
		}
		return result;
	}


	template<typename U>
	explicit operator const Vector<D, U>() const {
		Vector<D, U> results;
		for (size_t i = 0u; i < D; ++i) {
			results[i] = static_cast<U>(this->components[i]);
		}
		return results;
	}

	explicit operator const std::string() const {
		std::string r = std::to_string(this->components[0]);
		for (size_t i = 1u; i < D; ++i) {
			r += ' ';
			r += std::to_string(this->components[i]);
		}
		return r;
	}
#pragma endregion

	//SFML compatibility stuff
#ifdef SFML_VECTOR2_HPP
	template<typename U, size_t Dp = D>
	constexpr Vector(const sf::Vector2<U>& p, std::enable_if_t<Dp == 2, T>* = nullptr) :
		__vec_member<2, T>((T)p.x, (T)p.y)
	{}

	template<typename U>
	operator const sf::Vector2<U>() const {
		static_assert(D == 2);
		return {
			static_cast<U>(this->x),
			static_cast<U>(this->y)
		};
	}

	template<typename U = T>
	operator const std::enable_if_t<
		std::is_same_v<U, T> && std::is_floating_point_v<T> && D == 4,
		sf::Color
	>() const {
		return sf::Color(COLOR_UNROLL(
			((Vector<4U, std::uint8_t>)(this->operator*(255)))
		));
	}

	template<typename U>
	Vector<D, T>& operator=(const sf::Vector2<U>& other) {
		static_assert(D == 2);
		this->x = static_cast<T>(other.x);
		this->y = static_cast<T>(other.y);
		return *this;
	}

	template<typename U>
	bool operator==(const sf::Vector2<U>& other) const {
		static_assert(D == 2);
		return this->components[0] == other.x && this->components[1] == other.y;
	}
	template<typename U>
	bool operator!=(const sf::Vector2<U>& other) const {
		static_assert(D == 2);
		return !this->operator==(other);
	}
	template<typename U>
	Vector<2, T> operator-(const sf::Vector2<U>& other) const {
		static_assert(D == 2);
		Vector<2, T> result;
		result.x = this->x - other.x;
		result.y = this->y - other.y;
		return result;
	}

	Vector<D, T> productCW(const Vector<D, T>& other) const noexcept {
		Vector<D, T> result;
		for (size_t i = 0; i < D; ++i) {
			result[i] = this->components[i] * other[i];
		}
		return result;
	}
#endif
#ifdef SFML_GRAPHICS_HPP

	static void renderLine(
		sf::RenderTarget& target,
		Vector<2U, T> A,
		Vector<2U, T> B,
		Vector4d colorA,
		Vector4d colorB
	) {
		sf::Vertex vertex[2]{
			sf::Vertex(A, (sf::Color)colorA),
			sf::Vertex(B, (sf::Color)colorB)
		};

		target.draw(vertex, 2, sf::Lines);
	}
	static void renderLine(
		sf::RenderTarget& target,
		Vector<2U, T> A,
		Vector<2U, T> B,
		Vector4d colorA
	) {
		Vector<2U, T>::renderLine(target, A, B, colorA, colorA);
	}

	void render(
		sf::RenderTarget& target, Vector4d colorA, Vector4d colorB = colorA
	) const {
		Vector<2U, T>::renderLine(target, { (T)0, (T)0 }, *this, colorA, colorB);
	}
	void render(sf::RenderTarget& target, Vector4d colorA) const {
		render(target, colorA, colorA);
	}

	void plot(
		sf::RenderTarget& target,
		float r,
		Vector4d fill,
		Vector4d outline,
		float thick
	) const {
		sf::CircleShape circle(r);
		circle.setOrigin(r, r);
		circle.setPosition((float)this->x, (float)this->y);
		circle.setFillColor(fill);
		circle.setOutlineColor(outline);
		circle.setOutlineThickness(thick);
		target.draw(circle);
	}

	void drawArrow(
		sf::RenderTarget& target, float thick, Vector4d color, Vector2<T> offset = { 0, 0 }
	) const noexcept {
		sf::RectangleShape stick{ sf::Vector2f{ (float)length() * 0.9f, thick * 2 } };
		stick.setOrigin(0, thick);
		stick.setPosition(offset);
		stick.setRotation((float)(angleX() * Common::RAD_2_DEG));

		sf::CircleShape triangle{ 3 * thick * std::tanf((float)Common::PI / 3.f), 3 };
		triangle.setOrigin(triangle.getRadius(), triangle.getRadius());
		triangle.setRotation(stick.getRotation() + 90);
		triangle.setPosition(
			(Vector2f)offset +
			(Vector2f::createUnitVector(angleX()) *
			((float)length() * 0.9f - triangle.getRadius() * (1 - 1 / 3.f)))
		);

		stick.setFillColor(color);
		triangle.setFillColor(color);

		target.draw(stick);
		target.draw(triangle);
	}

#endif

	template<size_t Dp = D>
	static std::enable_if_t<Dp == 3, std::optional<Vector3f>> intersect_tirangle(
		Vector<D, T> P, Vector<D, T> V, Vector<D, T> v0, Vector<D, T> v1, Vector<D, T> v2
	) noexcept {
		const float EPSILON = 0.0000001f;

		// compute plane's normal
		auto v0v1 = v1 - v0;
		auto v0v2 = v2 - v0;
		// no need to normalize
		auto N = v0v1.cross(v0v2); // N 
		float area2 = N.length();

		// Step 1: finding P

		// check if ray and plane are parallel ?
		float NdotRayDirection = N.dot(V);
		if (fabs(NdotRayDirection) < EPSILON) // almost 0 
			return std::nullopt; // they are parallel so they don't intersect ! 

		// compute d parameter using equation 2
		float d = N.dot(v0);

		// compute t (equation 3)
		auto t = (N.dot(P) + d) / NdotRayDirection;
		// check if the triangle is in behind the ray
		if (t < 0) return std::nullopt; // the triangle is behind 

		// compute the intersection point using equation 1
		auto p = P + t * V;

		// Step 2: inside-outside test
		Vector3f C; // vector perpendicular to triangle's plane 

		// edge 0
		auto edge0 = v1 - v0;
		auto vp0 = p - v0;
		C = edge0.cross(vp0);
		if (N.dot(C) < 0) return std::nullopt; // p is on the right side 

		// edge 1
		auto edge1 = v2 - v1;
		auto vp1 = p - v1;
		C = edge1.cross(vp1);
		if (N.dot(C) < 0)  return std::nullopt; // p is on the right side 

		// edge 2
		auto edge2 = v0 - v2;
		auto vp2 = p - v2;
		C = edge2.cross(vp2);
		if (N.dot(C) < 0) return std::nullopt; // p is on the right side; 

		return p; // this ray hits the triangle 
		/*
		Vector3<T> edge1, edge2, h, s, q;
		float a, f, u, v;
		edge1 = B - A;
		edge2 = C - A;
		h = V.cross(edge2);
		a = edge1.dot(h);
		if (a > -EPSILON && a < EPSILON)
			return std::nullopt;    // This ray is parallel to this triangle.
		f = 1.f / a;
		s = P - A;
		u = f * (s.dot(h));
		if (u < 0.0 || u > 1.0)
			return std::nullopt;
		q = s.cross(edge1);
		v = f * V.dot(q);
		if (v < 0.0 || u + v > 1.0)
			return std::nullopt;
		// At this stage we can compute t to find out where the intersection point is on the line.
		float t = f * edge2.dot(q);
		return (t > EPSILON) ? std::optional<Vector3f>{P + V * t} : std::nullopt; // ray intersection
		*/
	}

	template<size_t Dp = D>
	static std::enable_if_t<Dp == 3, std::optional<Vector3f>> ray_intersect_sphere(
		Vector3<T> C, T R, Vector3<T> ray_pos, Vector3<T> ray_dir 
	) noexcept {
		Vector3<T> m = ray_pos - C;
		float b = m.dot(ray_dir);
		float c = m.dot(m) - R * R;

		// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0) 
		if (c > 0.0f && b > 0.0f) return std::nullopt;
		float discr = b * b - c;

		// A negative discriminant corresponds to ray missing sphere 
		if (discr < 0.0f) return std::nullopt;

		// Ray now found to intersect sphere, compute smallest t value of intersection
		auto t = -b - sqrt(discr);

		// If t is negative, ray started inside sphere so clamp t to zero 
		if (t < 0.0f) t = 0.0f;
		return ray_pos + t * ray_dir;
	}

	//From https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, T> dist_from_line(Vector2<T> A, Vector2<T> dir) const noexcept {
		auto B = A + dir;
		
		return (Vector2f{ this->x, -this->y }.dot(B - A) + Vector2f{ A.x, -A.y }.dot(B)) /
				(B - A).length();
	}
};

template<size_t D, typename T, typename U>
Vector<D, T> operator*(U scalar, const Vector<D, T>& vec) {
	return vec * scalar;
}
template<size_t D, typename T, typename U>
Vector<D, T> operator/(U scalar, const Vector<D, T>& vec) noexcept {
	Vector<D, T> result;
	for (auto i = 0; i < D; ++i) {
		result[i] = scalar / vec[i];
	}
	return result;
}

namespace std {
	template<size_t D, typename T>
	struct hash<Vector<D, T>> {
		std::size_t operator()(const Vector<D, T>& k) const {
			auto seed = 0;
			for (size_t i = 0; i < D; ++i) {
				seed = xstd::hash_combine(seed, k[i]);
			}
			return seed;
		}
	};
}
/*
struct dyn_struct;
template<typename T>
void to_dyn_struct(dyn_struct& s, const Vector<2, T>& x) noexcept {
	s = { x.x, x.y };
	s.type_tag = Vector2_Type_Tag;
}
template<typename T>
void from_dyn_struct(const dyn_struct& s, Vector<2, T>& x) noexcept {
	x.x = (T)s[0];
	x.y = (T)s[1];
}
*/
#pragma warning(pop)