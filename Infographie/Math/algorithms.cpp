#include "algorithms.hpp"

#include <random>

#include "Containers/QuadTree.hpp"
#include "Common.hpp"


bool is_in_ellipse(Vector2f A, Vector2f B, float small_axis, Vector2f p) noexcept {
	return	(p - A).length() + (p - B).length() <=
			(B - A).length() + small_axis;
}
double evaluate_expression(const std::string& str) noexcept {
	// TODO

	return std::stod(str);
}

std::optional<Vector2f> segment_rec(const Segment2f& seg, const Rectangle2f& rec) noexcept {
	if (auto x = segment_segment(seg, { rec.pos, {rec.x, rec.y + rec.h} }))
		return x;
	if (auto x = segment_segment(seg, { {rec.x, rec.y + rec.h}, rec.pos + rec.size }))
		return x;
	if (auto x = segment_segment(seg, { rec.pos + rec.size, {rec.x + rec.w, rec.y} }))
		return x;
	if (auto x = segment_segment(seg, { {rec.x + rec.w, rec.y}, rec.pos }))
		return x;
	return {};
}

std::optional<Vector2f> segment_segment(const Segment2f& A, const Segment2f& B) noexcept {
	auto s1 = A.B - A.A;
	auto s2 = B.B - B.A;

	float det = (-s2.x * s1.y + s1.x * s2.y);
	if (det == 0) return {};

	float s, t;
	s = (-s1.y * (A.A.x - B.A.x) + s1.x * (A.A.y - B.A.y)) / det;
	t = (s2.x * (A.A.y - B.A.y) - s2.y * (A.A.x - B.A.x)) / det;

	if (std::max({ s, t, 1.f }) != 1 || std::min({ s, t, 0.f }) != 0) return {};

	return A.A + t * s1;
}

std::optional<Vector2f> ray_circle(const Rayf& ray, const Circlef& c) noexcept {
	auto e = Vector2f::createUnitVector(ray.angle);
	auto h = c.c - ray.pos;
	auto lf = e.dot(h);
	auto s = (c.r * c.r) - h.dot(h) + lf * lf;

	if (s < 0.0)	return std::nullopt;
	else			return e * (lf - std::sqrt(s)) + ray.pos;
}

std::optional<Vector2f> ray_rectangle(const Rayf& ray, const Rectangle2f& rec) noexcept {
	std::optional<Vector2f> x = std::nullopt;
	
	if (ray.pos.x < rec.x)
		x = ray_segment(ray, { rec.pos, {rec.x, rec.y + rec.h} });
	else if (ray.pos.x > rec.x + rec.size.x)
		x = ray_segment(ray, { rec.pos + rec.size, {rec.x + rec.w, rec.y} });

	if (ray.pos.y < rec.y)
		x = ray_segment(ray, { {rec.x + rec.w, rec.y}, rec.pos });
	else if (ray.pos.y > rec.y + rec.size.y)
		x = ray_segment(ray, { {rec.x, rec.y + rec.h}, rec.pos + rec.size });
	
	return x;
}

std::optional<Vector2f> ray_segment(const Rayf& A, const Segment2f& B) noexcept {
	auto s1 = Vector2f::createUnitVector(A.angle);
	auto s2 = B.B - B.A;

	float det = (-s2.x * s1.y + s1.x * s2.y);
	if (det == 0) return {};

	float s, t;
	s = (-s1.y * (A.pos.x - B.A.x) + s1.x * (A.pos.y - B.A.y)) / det;
	t = (s2.x * (A.pos.y - B.A.y) - s2.y * (A.pos.x - B.A.x)) / det;

	if (std::max({ s, 1.f }) != 1 || std::min({ s, t, 0.f }) != 0) return {};

	return A.pos + t * s1;
}

std::vector<Vector2f> poisson_disc_sampling(
	float r, Vector2f size, const std::vector<Vector2f>& initial_pool
) noexcept
{
	std::default_random_engine rng(SEED);
	constexpr double sqrt1_2 = 0.7071067811865476;

	size_t k = 20;
	double r2 = r * r;
	double R = 3 * r2;
	std::vector<Vector2f> queue;
	std::vector<Vector2f> poisson;
	QuadTree<25u, Vector2f, Vector2f, float> quad{ Rectangle2f{ { 0.f, 0.f }, size } };

	const auto sample = [&queue, &poisson, &quad](Vector2f p) -> void {
		queue.push_back(p);
		poisson.push_back(p);
		quad.add(p);
	};

	sample(Vector2f::rand({ 0, 0 }, size, rng));
	for (const auto& p : initial_pool) sample(p);

	auto aRange = std::uniform_real_distribution<>(0, 2 * PIf);
	auto tRange = std::uniform_real_distribution<>(0, R);

	decltype(quad)::vector query;
	std::vector<const decltype(quad)*> open;

	query.reserve((size_t)std::ceil((size.x / r) * (size.y / r) * 1.5));
	open.reserve(query.size() / decltype(quad)::bucket_size);

	while (true) {
		while (!queue.empty()) {
			size_t i = std::uniform_int_distribution<size_t>(0, queue.size() - 1)(rng);
			auto s = queue[i];

			for (size_t j = 0u; j < k; ++j) {
				double a = aRange(rng);
				double t = std::sqrt(tRange(rng) + r2);
				auto p = s + t * Vector2d::createUnitVector(a);

				if (0 <= p.x && p.x <= size.x &&
					0 <= p.y && p.y <= size.y
					) {
					query.resize(0);
					open.resize(0);
					quad.noAllocQueryCircle({ p, r }, query, open);

					bool flag = true;
					for (auto q : query) {
						if (Vector2d::equal(p, q, r)) {
							flag = false;
							break;
						}
					}
					if (flag) {
						sample(p);
					}
				}
			}

			queue[i] = queue.back();
			queue.pop_back();
		}
		break;
	}

	return poisson;
}

