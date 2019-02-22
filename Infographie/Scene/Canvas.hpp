#pragma once

#include <SFML/Graphics.hpp>
#include "Widget.hpp"

class Canvas : public Widget {
public:
	static size_t Total_N;
	Canvas() noexcept;

	void update(float dt) noexcept override;
	void render(sf::RenderTarget& target) noexcept override;

	void set_size(const Vector2f& size) noexcept override;
	void set_size(const Vector2u& size) noexcept;

	void paint_pixels(const std::vector<Vector2u>& pixels, Vector4f color) noexcept;
	sf::Image& get_image() noexcept;

	size_t get_n() const noexcept;
private:
	size_t n{ 0 };

	bool texture_cached{ false };

	sf::Sprite sprite;
	sf::Texture texture;
	sf::Image image;
};
