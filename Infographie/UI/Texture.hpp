#pragma once

#include <string>
#include <functional>
#include <filesystem>
#include <shared_mutex>

#include "Math/Vector.hpp"

#include "Utils/UUID.hpp"
#include "Scene/Widget.hpp"

struct Texture_Settings {
	static constexpr const char* Tone_String[] = {
		"Identity",
		"BW",
		"Sepia",
		"Edge",
		"Blur",
		"Count"
	};
	std::shared_mutex mutex;

	enum class Tone : int {
		Identity = 0,
		BW,
		Sepia,
		Edge,
		Blur,
		Count
	} current_tone{ Tone::Identity };

	float bw_leakage{ 0.f };
	float edge_threshold{ 0.5f };
	float blur_radius{ 0.01f };

	// i know, should be a matrix, but i don't want to templatize my Matrix4f just for that
	Vector3<Vector3f> sepia_weights{
		Vector3f{.393f, .769f, .189f},
		Vector3f{.349f, .686f, .168f},
		Vector3f{.272f, .534f, .131f}
	};

	std::pair<std::string, std::string> get_shader_path() const noexcept;

	std::vector<std::function<void(std::filesystem::path)>> cubemap_added;

	Widget* root{ nullptr };
	std::vector<Uuid_t> cubemap_ids;

	Vector4f gradient_color_start{ 0, 0, 0, 0 };
	Vector4f gradient_color_end{ 1, 1, 1, 1 };

	sf::Texture gradient_texture;
	sf::Image gradient_image;
};

extern void update_texture_settings(Texture_Settings& settings) noexcept;
