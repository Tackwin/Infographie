#pragma once
#include <vector>

#include "Scene/Camera.hpp"
#include "Utils/UUID.hpp"

struct Camera_Settings {
	Widget* root{ nullptr }; //< as usual this need to be alive for the whole program.

	std::vector<Uuid_t> camera_ids;
};

extern void update_camera_settings(Camera_Settings& settings) noexcept;

