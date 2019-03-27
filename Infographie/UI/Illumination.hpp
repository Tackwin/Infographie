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
		Vector3f color{ 1, 1, 1 };
	} ambient;

	struct Directional {
		Vector3f dir{ 0, -1, 0 };
		Vector3f color{ 1, 0.9f, 0.9f };
		float strength{ 0.1f };
	};

	std::vector<Uuid_t> light_point_ids;
	std::vector<Directional> directionals;
};

extern void update_illumination_settings(Illumination_Settings& settings) noexcept;

