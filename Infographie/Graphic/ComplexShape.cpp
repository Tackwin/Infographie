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


void Heart_Shape::draw(sf::RenderTarget& target, sf::RenderStates state) const noexcept {
	state.transform *= getTransform();

	// For the heart we have the render order:
	// - back triangle
	// - two half-circle with outline
	// - front triangle

	// the triangle will go (from top to bottom) 25 - 100
	// and span the whole width

	// the circle will be calculated from the distance of (-0.25, -0.25) from the left side
	// of the triangle

	// No thickness in the top because we don't want pointy triangle to exceed the circle's heart.
	sf::ConvexShape back_triangle;
	back_triangle.setPointCount(3);
	back_triangle.setPoint(0, { -(thick * 2 + size.x) / 2 , -0.25f * size.y });
	back_triangle.setPoint(1, { +(size.x + thick * 2) / 2, -0.25f * size.y });
	back_triangle.setPoint(2, { 0, 0.5f * (size.y + thick * 2) });
	back_triangle.setFillColor(outline_color);
	target.draw(back_triangle, state);

	// the circle will have a radius of 25% of the width
	sf::ConvexShape back_circle;
	back_circle.setPointCount(30);
	for (size_t i = 0; i < back_circle.getPointCount(); ++i) {
		auto r = (size.x / 4 + thick);
		back_circle.setPoint(
			i, r * Vector2f::createUnitVector(-(PIf * i) / (back_circle.getPointCount() - 1))
		);
	}
	back_circle.setFillColor(outline_color);
	back_circle.setPosition({ -0.25f * size.x, -0.25f * size.y });
	target.draw(back_circle, state);
	back_circle.setPosition({ 0.25f * size.x, -0.25f * size.y });
	target.draw(back_circle, state);

	sf::ConvexShape circle;
	circle.setPointCount(30);
	for (size_t i = 0; i < circle.getPointCount(); ++i) {
		auto r = (size.x / 4);
		circle.setPoint(
			i, r * Vector2f::createUnitVector(-(PIf * i) / (circle.getPointCount() - 1))
		);
	}
	circle.setFillColor(color);
	circle.setPosition({ -0.25f * size.x, -0.25f * size.y });
	target.draw(circle, state);
	circle.setPosition({ 0.25f * size.x, -0.25f * size.y });
	target.draw(circle, state);

	sf::ConvexShape triangle;
	triangle.setPointCount(3);
	triangle.setPoint(0, { -size.x / 2 , -0.25f * size.y });
	triangle.setPoint(1, { +size.x / 2, -0.25f * size.y });
	triangle.setPoint(2, { 0, 0.5f * size.y });
	triangle.setFillColor(color);
	target.draw(triangle, state);
}

Vector2f Heart_Shape::get_size() const noexcept {
	return size;
}
