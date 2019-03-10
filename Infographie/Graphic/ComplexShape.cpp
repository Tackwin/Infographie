#include "ComplexShape.hpp"


void Arrow_Shape::draw(sf::RenderTarget& target, sf::RenderStates state) const noexcept {
	state.transform *= getTransform();

	sf::RectangleShape outline_body;
	Vector2f outline_body_size = { 2 * (length / 3 + outline_thick), thick / 2 + outline_thick };
	outline_body.setFillColor((sf::Color)outline_color);
	outline_body.setSize(outline_body_size);
	outline_body.setOrigin(outline_body_size / 2);

	// first the outline of the main retangle.
	target.draw(outline_body, state);

	sf::ConvexShape tip;
	tip.setPointCount(3);
	tip.setPoint(0, { 0.5f * length / 3, thick });
	tip.setPoint(1, { 1.5f * length / 3, 0 });
	tip.setPoint(2, { 0.5f * length / 3, -thick });
	tip.setFillColor((sf::Color)color);
	tip.setOutlineColor((sf::Color)outline_color);
	tip.setOutlineThickness(outline_thick);

	// then the tip with it's outline.
	target.draw(tip, state);

	sf::RectangleShape body;
	body.setFillColor((sf::Color)color);
	body.setSize({ 2 * length / 3, thick / 2 });
	body.setOrigin(Vector2f{ 2 * length / 3, thick / 2 } / 2);

	// finally the body without outline.
	target.draw(body, state);
}

Vector2f Arrow_Shape::get_size() const noexcept {
	return { length, thick };
}
