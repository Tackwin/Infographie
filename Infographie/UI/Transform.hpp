#pragma once
#include "Scene/Widget.hpp"

struct Transform_Settings {
	Widget* root{ nullptr };
};

extern void update_transform_settings(const Transform_Settings& settings) noexcept;

