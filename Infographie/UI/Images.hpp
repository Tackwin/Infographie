#pragma once
#include <shared_mutex>
#include <functional>
#include <filesystem>
#include <vector>

#include "Utils/UUID.hpp"
#include "Scene/Widget.hpp"

struct Images_Settings {
	std::shared_mutex mutex;
	bool closed{ false };

	// _MUST_ be pointer to function only, no callable object as those might have
	// state that are not garantee to be accessible in the futur or thread safe
	// or else it's the callback responsability to remember that this code might execute
	// in another thread.

	// i'm not using c style function pointer because for one, it's fucking ulgy and for two
	// i keep a pointer to an instance of Images_Settings (this class) that _I MAKE SURE_
	// i synchronise it.
	std::vector<std::function<void(const std::filesystem::path&)>> import_images_callback;

	// that we don't care it's not gonna be async... though i might put that thing in another
	// thread since i fully expect it to take a long time... Humm maybe later
	// Tackwin 06-03
	std::vector<std::function<void(const sf::Image&)>> create_images_callback;

	Widget* root{ nullptr };
	std::vector<Uuid_t> images_widget_id;

	bool take_screenshot{ false };
	std::filesystem::path screenshot_directory;

	std::vector<std::string> log;
};

extern void update_image_settings(Images_Settings& settings) noexcept;

