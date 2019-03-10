#pragma once
#include <SFML/Graphics.hpp>
#include "Math/Vector.hpp"

struct Drawable_Transform : public sf::Drawable, public sf::Transformable {
	virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const override = 0;
};

// Since i'm going to be updating the underlying tip and body shape every draw call
// let's just make it a POD
struct Arrow_Shape : public Drawable_Transform {
	float length{ 0 };
	float thick{ 0 };
	Vector4f color{ 0, 0, 0, 1 };
	Vector4f outline_color{ 1, 1, 1, 0 };
	float outline_thick{ 0 };

	virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const noexcept override;
};
