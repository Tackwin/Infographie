#pragma once
#include "Math/Vector.hpp"
#include <vector>

#include "Utils/UUID.hpp"

class Model;
class Widget;
struct Illumination_Settings {

	Widget* root{nullptr};

	struct Ambient {
		float strength{ 0.1f };
		Vector4f color;
	} ambient;

	std::vector<Uuid_t> light_point_ids;
};

extern void update_illumination_settings(Illumination_Settings& settings) noexcept;

