#pragma once

class Widget;
struct Ray_Tracing_Settings {
	size_t blur_radius{ 2 };
	float ssao_radius{ 0.01f };
	float ssao_bias{ 0.025f };
	float noise_scale{ 4.0 };
	
	size_t kernel_sample_count{ 64 };

	Widget* root{ nullptr };
};

extern void update_ray_tracing_settings(Ray_Tracing_Settings& settings) noexcept;
