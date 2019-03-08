#include "Texture.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "OS/OpenFile.hpp"
#include "Scene/CubeMap.hpp"

std::pair<std::string, std::string> Texture_Settings::get_shader_path() const noexcept {
	std::string fragment;
	switch (current_tone)
	{
	case Texture_Settings::Tone::Identity:
		fragment = "res/shaders/Identity.fragment";
		break;
	case Texture_Settings::Tone::BW:
		fragment = "res/shaders/Black_And_White.fragment";
		break;
	case Texture_Settings::Tone::Sepia:
		fragment = "res/shaders/Sepia.fragment";
		break;
	case Texture_Settings::Tone::Edge:
		fragment = "res/shaders/Edge.fragment";
		break;
	case Texture_Settings::Tone::Blur:
		fragment = "res/shaders/Blur.fragment";
		break;
	default:
		assert(false);
		break;
	}
	return { "res/shaders/Identity.vertex", fragment };
}


void update_texture_settings(Texture_Settings& settings) noexcept {
	if (settings.gradient_image.getSize().x == 0) {
		settings.gradient_image.create(100, 100);
		settings.gradient_texture.create(100, 100);
	}

	if (ImGui::Button("Load Cubemap")) {
		open_dir_async([&](std::optional<std::filesystem::path> opt_path) {
			if (!opt_path) return;
			std::lock_guard guard{ settings.mutex };

			for (auto& f : settings.cubemap_added) f(*opt_path);
		});
	}
	if (settings.root) {
		ImGui::Separator();
		
		static int active_cubemap{ 0 };
		ImGui::ListBox(
			"Active cubemap",
			&active_cubemap,
			[](void* data, int idx, const char** out) -> bool {
				auto settings = (Texture_Settings*)data;
				auto cube_map = (Cube_Map*)settings->root->find_child(settings->cubemap_ids[idx]);
				if (!cube_map) return false;

				*out = cube_map->get_name().c_str();
				return true;
			},
			&settings,
			(int)settings.cubemap_ids.size()
		);
		int i = 0;
		for (auto& id : settings.cubemap_ids) {
			bool visible = i == active_cubemap;
			((Cube_Map*)settings.root->find_child(id))->set_visible(visible);
			++i;
		}
	}

	ImGui::Separator();

	ImGui::ListBox(
		"Tone mapping",
		(int*)&settings.current_tone,
		settings.Tone_String,
		(int)Texture_Settings::Tone::Count
	);

	ImGui::Separator();
	ImGui::DragFloat("Alpha tolerance", &Alpha_Tolerance, 0.01f, 0, 1);

	switch (settings.current_tone)
	{
	case Texture_Settings::Tone::Identity:
		break;
	case Texture_Settings::Tone::BW:
		ImGui::DragFloat("Leakage", &settings.bw_leakage, 0.01f, 0, 1);
		break;
	case Texture_Settings::Tone::Sepia:
		ImGui::DragFloat3("Sepia Red", (float*)&settings.sepia_weights.x, 0.01f, 0, 1);
		ImGui::DragFloat3("Sepia Blue", (float*)&settings.sepia_weights.y, 0.01f, 0, 1);
		ImGui::DragFloat3("Sepia Green", (float*)&settings.sepia_weights.z, 0.01f, 0, 1);
		break;
	case Texture_Settings::Tone::Edge:
		ImGui::DragFloat("Edge treshold", &settings.edge_threshold, 0.01f, 0, 1);
		break;
	case Texture_Settings::Tone::Blur:
		ImGui::DragFloat("Blur radius", &settings.blur_radius, 0.0001f, 0, 0.1f);
		break;
	default:
		break;
	}

	ImGui::Separator();

	ImGui::ColorEdit4("Gradient start", &settings.gradient_color_start.x);
	ImGui::ColorEdit4("Gradient end", &settings.gradient_color_end.x);
	if (ImGui::Button("Apply")) {
		for (size_t x = 0; x < settings.gradient_image.getSize().x; ++x) {
			for (size_t y = 0; y < settings.gradient_image.getSize().y; ++y) {
				float t = ((x * x) + (y * y)) / Vector2f{
					(float)settings.gradient_image.getSize().x,
					(float)settings.gradient_image.getSize().y
				}.length2();

				auto vec_color = 255 *
					(settings.gradient_color_start * (1 - t) + t * settings.gradient_color_end);
				settings.gradient_image.setPixel(x, y, sf::Color{
					(sf::Uint8)vec_color.x,
					(sf::Uint8)vec_color.y,
					(sf::Uint8)vec_color.z,
					(sf::Uint8)vec_color.w
				});
			}
		}
		
		settings.gradient_texture.update(settings.gradient_image.getPixelsPtr());
	}
	ImGui::Image(settings.gradient_texture, { 100, 100 });
	ImGui::Separator();
}
