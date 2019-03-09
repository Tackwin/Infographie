#include "Canvas.hpp"
#include "Managers/InputsManager.hpp"

#include "Window.hpp"

size_t Canvas::Total_N{ 0 };

Canvas::Canvas(const Drawing_Settings& settings) noexcept : settings(settings) {
	Total_N++;
	n = Total_N;

	sprite.setTexture(texture);

	on_click.began = [&]() {
		click_cursor = push_cursor(sf::Cursor::Hand);
		return true;
	};
	on_click.going = [&]() {
		if (IM::isMousePressed(sf::Mouse::Button::Middle)) {
			auto dt = IM::getMouseScreenDelta();
			set_global_position(get_global_position() + dt);
			return true;
		}
		else if (IM::isMousePressed(sf::Mouse::Button::Left)) {
			action_on_canvas();
			return true;
		}
		return false;
	};
	on_click.ended = [&]() {
		pop_cursor(click_cursor);
		return true;
	};

	on_hover.began = [&]() {
		mouse_is_in_canvas = true;
		hover_cursor = push_cursor(sf::Cursor::Cross);
		return true;
	};
	on_hover.going = [&]() {
		return true;
	};
	on_hover.ended = [&]() {
		mouse_is_in_canvas = false;
		pop_cursor(hover_cursor);
		return true;
	};
}

void Canvas::update(float) noexcept {
	sprite.setPosition(get_global_position());
}

void Canvas::render(sf::RenderTarget& target) noexcept {
	auto last_view = target.getView();
	target.setView(target.getDefaultView());

	if (!texture_cached) {
		defer{ texture_cached = true; };
	
		texture.loadFromImage(image);
		sprite = sf::Sprite{ texture };
		sprite.setPosition(get_global_position());
	}

	target.draw(sprite);

	if (line_start_point) {
		constexpr auto radius = 5.f;
		sf::CircleShape marker{ radius };
		marker.setOrigin(radius, radius);
		marker.setPosition(*line_start_point + get_global_position());
		marker.setFillColor(sf::Color{ 100, 150, 200, 125 });
		target.draw(marker);
	}

#ifndef NDEBUG

	sf::RectangleShape border;
	border.setPosition(get_global_position());
	border.setSize(get_size());
	border.setOutlineThickness(1);
	border.setOutlineColor(Vector4f{ 1, 0, 0, 1 });
	border.setFillColor(Vector4f{ 0, 0, 0, 0 });

	target.draw(border);
#endif

	target.setView(last_view);
}

void Canvas::set_size(const Vector2f& s) noexcept {
	uint8_t* pixels = new uint8_t[(size_t)s.x * (size_t)s.y * 4];
	auto old_pixels = image.getPixelsPtr();
	defer{ delete[] pixels; };

	memset(pixels, 255, (size_t)s.x * (size_t)s.y * 4);
	for (size_t x = 0; x < 4 * (size_t)std::min(s.x, size.x); x += 4) {
		for (size_t y = 0; y < 4 * (size_t)std::min(s.y, size.y); y += 4) {
			pixels[(x + y * (size_t)s.y) + 0] = old_pixels[(x + y * image.getSize().y) + 0];
			pixels[(x + y * (size_t)s.y) + 1] = old_pixels[(x + y * image.getSize().y) + 1];
			pixels[(x + y * (size_t)s.y) + 2] = old_pixels[(x + y * image.getSize().y) + 2];
			pixels[(x + y * (size_t)s.y) + 3] = old_pixels[(x + y * image.getSize().y) + 3];
		}
	}

	Widget::set_size(s);
	image.create(UNROLL_2_P(s, size_t), pixels);
	texture_cached = false;
}
void Canvas::set_size(const Vector2u& s) noexcept {
	set_size(Vector2f{ UNROLL_2_P(s, float) });
}

void Canvas::paint_pixels(const std::vector<Vector2u>& pixels, Vector4f color) noexcept {
	for (auto& x : pixels) {
		image.setPixel(x.x, x.y, color);
	}
}

sf::Image& Canvas::get_image() noexcept {
	return image;
}

size_t Canvas::get_n() const noexcept {
	return n;
}

// Algorithm from https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void Canvas::fill_circle(Vector2u p, size_t radius, Vector4f color) noexcept {
	int x = (int)radius - 1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (radius << 1);

	while (x >= y) {
		fill_line(p, {p.x + x, p.y + y}, 1, color);
		fill_line(p, {p.x + y, p.y + x}, 1, color);
		fill_line(p, {p.x - y, p.y + x}, 1, color);
		fill_line(p, {p.x - x, p.y + y}, 1, color);
		fill_line(p, {p.x - x, p.y - y}, 1, color);
		fill_line(p, {p.x - y, p.y - x}, 1, color);
		fill_line(p, {p.x + y, p.y - x}, 1, color);
		fill_line(p, {p.x + x, p.y - y}, 1, color);

		if (err <= 0) {
			y++;
			err += dy;
			dy += 2;
		}

		if (err > 0) {
			x--;
			dx += 2;
			err += dx - (radius << 1);
		}
	}
}
void Canvas::fill_line(Vector2u A, Vector2u B, size_t thick, Vector4f color) noexcept {
	auto low = [&](Vector2u A, Vector2u B, Vector4f color) {
		auto dt = (Vector2i)B - (Vector2i)A;
		auto yi = 1;
		if (dt.y < 0) {
			yi = -1;
			dt.y = -dt.y;
		}
		auto D = 2 * dt.y - dt.x;
		auto y = A.y;

		for (size_t x = A.x; x < B.x; ++x) {
			image.setPixel(x, y, (sf::Color)color);
			if (D > 0) {
				y = y + yi;
				D = D - 2 * dt.x;
			}
			D = D + 2 * dt.y;
		}
	};
	auto high = [&](Vector2u A, Vector2u B, Vector4f color) {
		auto dt = (Vector2i)B - (Vector2i)A;
		auto xi = 1;
		if (dt.x < 0) {
			xi = -1;
			dt.x = -dt.x;
		}
		auto D = 2 * dt.x - dt.y;
		auto x = A.x;

		for (size_t y = A.y; y < B.y; ++y) {
			image.setPixel(x, y, (sf::Color)color);
			if (D > 0) {
				x = x + xi;
				D = D - 2 * dt.y;
			}
			D = D + 2 * dt.x;
		}
	};
	if (abs((int)B.y - (int)A.y) < std::abs((int)B.x - (int)A.x)) {
		if (A.x > B.x) low(B, A, color);
		else low(A, B, color);
	}
	else {
		if (A.y > B.y) high(B, A, color);
		else high(A, B, color);
	}
}

void Canvas::flood_fill(Vector2u p, Vector4u color, float tolerance) noexcept {
	Vector4u og_color{ COLOR_UNROLL(image.getPixel(p.x, p.y)) };

	std::vector<Vector2u> open;
	open.push_back(p);

	auto test = [tolerance](Vector4u a, Vector4u b) mutable {
		// the max distance between two vectors.
		tolerance *= Vector3u{ 255, 255, 255 }.length2();

		return (b.rgb - a.rgb).length2() <= tolerance;
	};

	while (!open.empty()) {
		auto last = open.back();
		open.pop_back();

		Vector4u image_color{ COLOR_UNROLL(image.getPixel(last.x, last.y)) };

		if (image_color == color) continue;
		if (!test(og_color, image_color)) continue;

		image.setPixel(last.x, last.y, sf::Color{ COLOR_UNROLL_P(color, uint8_t) });

		if (last.x > 0)
			open.push_back(last + Vector2i{ -1, 0 });

		if (last.x + 1 < image.getSize().x)
			open.push_back(last + Vector2i{ +1, 0 });
	
		if (last.y > 0)
			open.push_back(last + Vector2i{ 0, -1 });

		if (last.y + 1 < image.getSize().y)
			open.push_back(last + Vector2i{ 0, +1 });
	}

}

void Canvas::fill_square(Vector2u center, size_t s, Vector4f color) noexcept {
	for (
		size_t x = std::max((int)center.x - (int)s / 2, 0);
		x < std::min(center.x + s / 2, image.getSize().x);
		++x
	) {
		for (
			size_t y = std::max((int)center.y - (int)s / 2, 0);
			y < std::min(center.y + s / 2, image.getSize().y);
			++y
		) {
			image.setPixel(x, y, color);
		}
	}
}


void Canvas::action_on_canvas() noexcept {
	std::visit([&](auto&& x){
		using type_t = std::decay_t<decltype(x)>;
		auto mouse_canvas_pos = (Vector2u)(IM::getMouseScreenPos() - get_global_position());

		if constexpr (std::is_same_v<type_t, Drawing_Settings::DT_Circle>) {
			fill_circle(mouse_canvas_pos, x.size, x.color);
			texture_cached = false;
		}
		if constexpr (std::is_same_v<type_t, Drawing_Settings::DT_Fill>) {
			flood_fill(mouse_canvas_pos, (Vector4u)(x.color * 255), x.tolerance);
			texture_cached = false;
		}
		if constexpr (std::is_same_v<type_t, Drawing_Settings::DT_Line>) {
			if (!IM::isMouseJustPressed(sf::Mouse::Left)) return;
			if (!line_start_point) {
				line_start_point = mouse_canvas_pos;
			}
			else {
				fill_line(*line_start_point, mouse_canvas_pos, 1, x.color);
				texture_cached = false;
				line_start_point.reset();
			}
		}
		if constexpr (std::is_same_v<type_t, Drawing_Settings::DT_Square>) {
			fill_square(mouse_canvas_pos, x.size, x.color);
			texture_cached = false;
		}

	}, settings.drawing_tool);
}
