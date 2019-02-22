#include "Canvas.hpp"
#include "Managers/InputsManager.hpp"

size_t Canvas::Total_N{ 0 };

Canvas::Canvas() noexcept {
	Total_N++;
	n = Total_N;

	sprite.setTexture(texture);

	on_click.going = [&]() {
		if (!IM::isMousePressed(sf::Mouse::Button::Middle)) return false;
		if (!get_global_bounding_box().in(IM::getMouseScreenPos())) return false;

		auto dt = IM::getMouseScreenDelta();
		set_global_position(get_global_position() + dt);
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
