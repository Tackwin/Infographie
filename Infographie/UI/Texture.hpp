#pragma once

#include <string>

#include "Math/Vector.hpp"

struct Texture_Settings {
	static constexpr const char* Tone_String[] = {
		"Identity",
		"BW",
		"Sepia",
		"Edge",
		"Blur",
		"Count"
	};
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
};

extern void update_texture_settings(Texture_Settings& settings) noexcept;
