#pragma once
#include "Widget.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class Primitive : public Widget {
public:
	Primitive(std::unique_ptr<sf::Shape> shape) noexcept;

	virtual void render(sf::RenderTarget& target) noexcept override;
	virtual void update(float dt) noexcept override;

private:
	std::unique_ptr<sf::Shape> shape;
	sf::Cursor* click_cursor;
};
