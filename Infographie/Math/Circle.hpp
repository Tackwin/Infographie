#pragma once

#include "Vector.hpp"
#include "Rectangle.hpp"

template<size_t D, typename T>
struct Circle {
	Vector<D, T> c;
	T r;
};
template<typename T>
struct Circle<2, T> {
	Vector2<T> c;
	T r;
};

template<typename T>
using Circle2 = Circle<2, T>;
using Circle2f = Circle<2, float>;
using Circle2d = Circle<2, double>;

template<typename T>
bool is_in(const Rectangle2<T>& rec, const Circle2<T>& c) noexcept {
	Vector2<T> circle_distance = (c.c - rec.center()).applyCW([](auto x) {return std::abs(x); });

	if (circle_distance.x > (rec.w / 2 + c.r)) { return false; }
	if (circle_distance.y > (rec.h / 2 + c.r)) { return false; }
	if (circle_distance.x <= (rec.w / 2)) { return true; }
	if (circle_distance.y <= (rec.h / 2)) { return true; }

	auto corner_distance_sq =
		(circle_distance.x - rec.w / 2) * (circle_distance.x - rec.w / 2) +
		(circle_distance.y - rec.h / 2) * (circle_distance.y - rec.h / 2);

	return corner_distance_sq <= c.r * c.r;
}
template<typename T>
bool is_in(const Rectangle_t<3, T>& rec, const Circle<3, T>& c) noexcept {
	Vector3<T> circle_distance = (c.c - rec.center()).applyCW([](auto x) {return std::abs(x); });

	if (circle_distance.x > (rec.size[0] / 2 + c.r)) { return false; }
	if (circle_distance.y > (rec.size[1] / 2 + c.r)) { return false; }
	if (circle_distance.z > (rec.size[2] / 2 + c.r)) { return false; }
	if (circle_distance.x <= (rec.size[0] / 2)) { return true; }
	if (circle_distance.y <= (rec.size[1] / 2)) { return true; }
	if (circle_distance.z <= (rec.size[2] / 2)) { return true; }

	auto corner_distance_sq =
		(circle_distance.x - rec.size[0] / 2) * (circle_distance.x - rec.size[0] / 2) +
		(circle_distance.y - rec.size[1] / 2) * (circle_distance.y - rec.size[1] / 2) +
		(circle_distance.z - rec.size[2] / 2) * (circle_distance.y - rec.size[2] / 2);

	return corner_distance_sq <= c.r * c.r;
}
template<typename T>
bool is_in(const Vector2<T>& vec, const Circle2<T>& c) noexcept {
	return  (c.c - vec).length2() < c.r * c.r;
}

template<typename T>
bool is_fully_in(const Rectangle2<T>& rec, const Circle2<T>& c) noexcept {
	Vector2<T> circle_distance = (c.c - rec.center()).applyCW([](auto x) {return std::abs(x); });

	if (circle_distance.x > (rec.w / 2 - c.r)) { return false; }
	if (circle_distance.y > (rec.h / 2 - c.r)) { return false; }
	return true;
}

template<typename T>
bool is_fully_in(const Rectangle_t<3, T>& rec, const Circle<3, T>& c) noexcept {
	Vector3<T> circle_distance = (c.c - rec.center()).applyCW([](auto x) {return std::abs(x); });

	if (circle_distance.x > (rec.size[0] / 2 - c.r)) { return false; }
	if (circle_distance.y > (rec.size[1] / 2 - c.r)) { return false; }
	if (circle_distance.z > (rec.size[2] / 2 - c.r)) { return false; }
	return true;
}
