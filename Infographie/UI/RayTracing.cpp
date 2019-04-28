#include "RayTracing.hpp"

#include "Scene/Camera.hpp"

#include "imgui/imgui.h"

void update_ray_tracing_settings(Ray_Tracing_Settings& settings) noexcept {

	int x = settings.blur_radius;
	ImGui::DragInt("Blur radius", &x, 1, 1, 16);
	settings.blur_radius = x;

	ImGui::DragFloat("SSAO radius", &settings.ssao_radius, 0.001f, 0.f, 1.f);
	ImGui::DragFloat("SSAO bias", &settings.ssao_bias, 0.001f, 0.f, 1.f);

	x = settings.kernel_sample_count;
	ImGui::DragInt("Kernel sample count", &x, 1, 32, 256);
	settings.kernel_sample_count = x;

	ImGui::DragFloat("Noise scale", &settings.noise_scale, 0.5f, 1.f, 10.f);

	if (!settings.root) return;

	settings.root->for_every_childs([&](Widget * w) {
		if (auto* c = dynamic_cast<Camera*>(w); c) {
			c->ray_tracing_settings = settings;
		}
	});
}
