#pragma once
#include <string>

#include <SFML/Graphics.hpp>

#include "Math/Vector.hpp"

namespace details {
	struct Window_Struct {
		static Window_Struct* instance;

		Vector2u size;
		std::string title;

		sf::RenderWindow window;
	};
}

#define Window_Info (*details::Window_Struct::instance)
