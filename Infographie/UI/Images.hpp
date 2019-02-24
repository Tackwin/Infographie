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
	std::vector<std::function<void(const std::filesystem::path&)>> import_images_callback;

	Widget* root{ nullptr };
	std::vector<Uuid_t> images_widget_id;

	bool take_screenshot{ false };
	std::filesystem::path screenshot_directory;

	std::vector<std::string> log;
};

extern void update_image_settings(Images_Settings& settings) noexcept;

