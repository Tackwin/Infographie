#pragma once
#include <shared_mutex>
#include <functional>
#include <filesystem>
#include <vector>

#include "Utils/UUID.hpp"

#include "Scene/Widget.hpp"

#include "Files/FileFormat.hpp"

struct Geometries_Settings {
	std::shared_mutex mutex;

	std::vector<std::function<void(const std::filesystem::path&)>> model_added_callback;
	std::vector<std::function<void(Uuid_t, const std::filesystem::path&)>> texture_added_callback;
	std::vector<std::function<void(const Object_File&)>> spawn_object_callback;

	Widget* root{ nullptr };
	std::vector<Uuid_t> models_widget_id;
};

extern void update_geometries_settings(Geometries_Settings& settings) noexcept;
