#include "Bezier.hpp"

#include <random>

#include "imgui/imgui.h"

#include "Managers/InputsManager.hpp"

Bezier::Bezier(size_t n) noexcept {
	Vector2f start{ -50, 0 };
	Vector2f end{ 50, 0 };

	points.push_back(start);

	for (size_t i = 0; i + 2 < n; ++i) {
		float t = (i + 1.f) / (n - 1);
		points.push_back(start + t * (end - start));
	}

	points.push_back(end);
}

void Bezier::update(float dt) noexcept {
	Widget::update(dt);
	if (!is_visible()) return;
	if (!open) return;

	static Vector2f last_bezier_pos;

	ImGui::PushID(this);
	defer{ ImGui::PopID(); };

	Vector2f true_min = points[0];
	Vector2f max = points[0] + Vector2f{ 25, 25 };
	for (auto& x : points) {
		true_min.x = true_min.x < x.x ? true_min.x : x.x;
		true_min.y = true_min.y < x.y ? true_min.y : x.y;
		max.x = max.x > x.x ? max.x : x.x;
		max.y = max.y > x.y ? max.y : x.y;
	}
	Vector2f min = true_min - Vector2f{ 25, 25 };
	ImGuiWindowFlags win_flags = ImGuiWindowFlags_None;

	auto mouse_pos = IM::getMouseScreenPos();
	Rectangle2f rec{ last_bezier_pos - Vector2f{5, 5}, max - min + Vector2f{10, 10} };
	if (rec.in(mouse_pos)) {
		win_flags |= ImGuiWindowFlags_NoMove;
	}

	if (IM::isMouseJustPressed(sf::Mouse::Left)) {
		for (size_t i = 0; i < points.size(); ++i) {
			auto global_point_pos = get_global_position() + points[i] - true_min;
			if ((mouse_pos - global_point_pos).length2() < 5 * 5) {
				dragged = i;
			}
		}
	}
	if (dragged && IM::isMouseJustReleased(sf::Mouse::Left)) {
		dragged = std::nullopt;
	}
	if (dragged) {
		points[*dragged] += IM::getMouseScreenDelta();
	}



	ImGui::Begin("Bezier Curve", &open, win_flags);
	defer{ ImGui::End(); };

	auto window_size = ImGui::GetWindowSize();

	auto bezier_pos = Vector2f{ ImGui::GetCursorPos() } + Vector2f{ ImGui::GetWindowPos() };

	ImGui::Dummy(max - min);
	set_global_position(bezier_pos);

	int x = (int)resolution;
	ImGui::DragInt("Resolution", &x, 1, 0, 100);
	resolution = x;

	ImGui::ColorEdit3("Color", &color.x);
	last_bezier_pos = bezier_pos;

	if (ImGui::Button("Randomize ! :D")) {
		static size_t N_Randomize{ 0 };

		std::uniform_real_distribution<float> dist(0, 250);
		std::default_random_engine eng(SEED + N_Randomize++);

		for (auto& p : points) {
			p.x = dist(eng);
			p.y = dist(eng);
		}
	}
}

void Bezier::render(sf::RenderTarget& target) noexcept {
	Widget::render(target);
	if (!open) return;
	if (!is_visible()) return;

	Vector2f min = points[0];
	for (auto& x : points) {
		min.x = min.x < x.x ? min.x : x.x;
		min.y = min.y < x.y ? min.y : x.y;
	}

	sf::CircleShape point_mark;
	point_mark.setRadius(5);
	point_mark.setOrigin(5, 5);
	point_mark.setFillColor(sf::Color::White);

	for (auto& x : points) {
		point_mark.setPosition(get_global_position() + x - min);
		target.draw(point_mark);
	}

	std::vector<sf::Vertex> vertices;
	vertices.reserve(resolution);

	for (size_t i = 0; i < resolution; ++i) {
		float t = i / (resolution - 1.f);

		sf::Vertex vertex;
		vertex.position = get_global_position() + get_point(t) - min;
		vertex.color = (sf::Color)color;
		vertex.color.a = 255;

		vertices.push_back(vertex);
	}

	target.draw(vertices.data(), vertices.size(), sf::LineStrip);
}

Vector2f Bezier::get_point(float t) noexcept {
	auto n = points.size() - 1;

	Vector2d point{ 0, 0 };

	for (size_t i = 0; i <= n; ++i) {
		double binomial = xstd::fact(n) / (double)(xstd::fact(n - i) * xstd::fact(i));

		Vector2d point_double = (Vector2d)points[i];
		point_double *= binomial * std::pow(1 - t, n - i) * std::pow(t, i);

		point += point_double;
	}

	return (Vector2f)point;
}
