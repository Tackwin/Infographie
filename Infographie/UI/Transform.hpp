#pragma once
#include "Scene/Widget.hpp"
#include <functional>
#include <vector>

struct Transform_Settings {
	Widget* root{ nullptr };

	struct Diff_Action {
		std::function<void()> undo;
		std::function<void()> redo;
	};
	std::vector<Diff_Action> undos;
	std::vector<Diff_Action> redos;
};

extern void update_transform_settings(Transform_Settings& settings) noexcept;

