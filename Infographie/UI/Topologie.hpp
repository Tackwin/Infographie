#pragma once
#include <vector>
#include <functional>

class Widget;
struct Topologie_Settings {
	size_t n_points{ 0 };

	std::vector<std::function<void(size_t)>> added_bezier;

	Widget* root{ nullptr };
};

extern void update_topologie_settings(Topologie_Settings& settings) noexcept;
