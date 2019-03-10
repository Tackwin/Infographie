#include "Primitive.hpp"

#include "Managers/InputsManager.hpp"
#include "Window.hpp"

size_t Primitive::N{ 0 };

Primitive::Primitive(std::unique_ptr<Complex_Shape> s) noexcept : complex_shape(std::move(s)) {
	n = N;
	N++;
	on_click.began = [&] {
		if (IM::isMouseJustPressed(sf::Mouse::Button::Middle)) {
			click_cursor = push_cursor(sf::Cursor::Hand);
		}
		return true;
	};
	on_click.going = [&] {
		if (IM::isMousePressed(sf::Mouse::Button::Middle)) {
			auto dt = IM::getMouseScreenDelta();
			set_global_position(get_global_position() + dt);
			return true;
		}
		return false;
	};

	on_hover.ended = [&] {
		// >SEE canvas.cpp Canvas::Canvas to know why we do that here instead of on_click.ended.
		if (click_cursor) {
			pop_cursor(click_cursor);
			click_cursor = nullptr;
		}
		return false;
	};

	set_size(complex_shape->get_size());
	set_origin({ 0.5f, 0.5f });
};

Primitive::Primitive(std::unique_ptr<sf::Shape> s) noexcept : shape(std::move(s)) {
	n = N;
	N++;
	on_click.began = [&] {
		if (IM::isMouseJustPressed(sf::Mouse::Button::Middle)) {
			click_cursor = push_cursor(sf::Cursor::Hand);
		}
		return true;
	};
	on_click.going = [&] {
		if (IM::isMousePressed(sf::Mouse::Button::Middle)) {
			auto dt = IM::getMouseScreenDelta();
			set_global_position(get_global_position() + dt);
			return true;
		}
		return false;
	};

	on_hover.ended = [&] {
		// >SEE canvas.cpp Canvas::Canvas to know why we do that here instead of on_click.ended.
		if (click_cursor) {
			pop_cursor(click_cursor);
			click_cursor = nullptr;
		}
		return false;
	};

	set_size({ shape->getGlobalBounds().width, shape->getGlobalBounds().height });
	set_origin({ 0.5f, 0.5f });
};
void Primitive::render(sf::RenderTarget& target) noexcept {
	if (!is_visible()) return;
	if (shape) {
		shape->setPosition(get_global_position());
		target.draw(*shape);
	}
	if (complex_shape) {
		complex_shape->setPosition(get_global_position());
		target.draw(*complex_shape);
	}
}

void Primitive::update(float) noexcept { }

size_t Primitive::get_n() const noexcept {
	return n;
}
