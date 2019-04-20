#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include "Utils/UUID.hpp"

#include "Widget.hpp"

class Bezier : public Widget {
public:
	Bezier(size_t n) noexcept;

	void update(float dt) noexcept override;
	void render(sf::RenderTarget& target) noexcept override;

	Vector2f get_point(float t) noexcept;

	size_t get_n_points() noexcept {
		return points.size();
	}

	bool is_open() const noexcept {
		return open;
	}
	void set_open(bool x) noexcept {
		open = x;
	}
private:
	bool open{ true };

	size_t resolution{ 100 };

	Vector4f color;

	bool mouse_clicked_in_bezier_area{ false };

	std::optional<size_t> dragged;

	std::vector<Vector2f> points;
};
