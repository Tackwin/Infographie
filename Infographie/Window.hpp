#pragma once
#include <string>
#include <memory>
#include <shared_mutex>
#include <SFML/Graphics.hpp>

#include "Math/Vector.hpp"
#include "Scene/Camera.hpp"

struct Illumination_Settings;
namespace details {
	struct Window_Struct {
		static Window_Struct* instance;

		// >TODO: REMOVE ME
		Illumination_Settings* ill_settings;

		std::shared_mutex cursor_mutex;

		Vector2u size;
		std::string title;

		Vector3f clear_color;
		sf::RenderWindow window;

		Camera* active_camera;

		std::vector<std::unique_ptr<sf::Cursor>> cursor_stack;
	};
}

#define Window_Info (*details::Window_Struct::instance)

extern sf::Cursor* push_cursor(sf::Cursor::Type type) noexcept;
extern void pop_cursor(sf::Cursor* cursor) noexcept;
