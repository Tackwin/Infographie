#pragma once
#include <string>

#include <SFML/Graphics.hpp>

#include "Math/Vector.hpp"
#include "Scene/Camera.hpp"

namespace details {
	struct Window_Struct {
		static Window_Struct* instance;

		Vector2u size;
		std::string title;

		Vector3f clear_color;
		sf::RenderWindow window;

		Camera* active_camera;
	};
}

#define Window_Info (*details::Window_Struct::instance)
