#pragma once
#include <SFML/Graphics.hpp>
#include "Widget.hpp"

struct Image : public Widget {
	static size_t Total_N;
	size_t n;

	bool open;

	sf::Sprite sprite;

	Image(sf::Texture& texture) noexcept;

	void update(float dt) noexcept override;
};
