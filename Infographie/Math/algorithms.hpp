#pragma once
#include <string>
#include <optional>

#include "Vector.hpp"
#include "Segment.hpp"
#include "Rectangle.hpp"
#include "Circle.hpp"
#include "Ray.hpp"

template<typename T>
T min_dist2(const Segment2<T>& seg, const Vector2<T>& p) noexcept {
	Vector2<T> v = seg.B - seg.A;
	Vector2<T> w = p - seg.A;

	T c1 = w.dot(v);
	if (c1 <= 0)
		return (p - seg.A).length2();

	T c2 = v.dot(v);
	if (c2 <= c1)
		return (p - seg.B).length2();

	T b = c1 / c2;
	return (p - (seg.A + b * v)).length2();
}
template<typename T>
T min_dist(const Segment2<T>& seg, const Vector2<T>& p) noexcept {
	return static_cast<T>(std::sqrt(min_dist2(seg, p)));
}
extern bool is_in_ellipse(Vector2f A, Vector2f B, float r, Vector2f p) noexcept;
extern double evaluate_expression(const std::string& str) noexcept;
extern
std::optional<Vector2f> segment_rec(const Segment2f& seg, const Rectangle2f& rec) noexcept;
extern
std::optional<Vector2f> segment_segment(const Segment2f& A, const Segment2f& B) noexcept;
extern
std::optional<Vector2f> ray_circle(const Rayf& ray, const Circlef& c) noexcept;
extern
std::optional<Vector2f> ray_rectangle(const Rayf& seg, const Rectangle2f& c) noexcept;
extern std::optional<Vector2f> ray_segment(const Rayf& A, const Segment2f& B) noexcept;

extern std::vector<Vector2f> poisson_disc_sampling(
	float r, Vector2f size, const std::vector<Vector2f>& initial_pool = {}
) noexcept;
