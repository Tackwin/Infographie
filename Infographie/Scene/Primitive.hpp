#pragma once
#include "Widget.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include "Graphic/ComplexShape.hpp"

class Primitive : public Widget {
public:

	// I didn't put much thinking into this but i think that thanks to the weird sfml class
	// hierarchy (namely there is no sf::Draw&Transform) and since shape is only for convex
	// shape i need to have two constructor end can't make a simple pointer
	// since i can't ask a pointer to be a union of two pure base virtual class.
	Primitive(std::unique_ptr<Complex_Shape> shape) noexcept;
	Primitive(std::unique_ptr<sf::Shape> shape) noexcept;

	virtual void render(sf::RenderTarget& target) noexcept override;
	virtual void update(float dt) noexcept override;

private:
	std::unique_ptr<sf::Shape> shape;
	std::unique_ptr<Complex_Shape> complex_shape;
	sf::Cursor* click_cursor{ nullptr };
};
