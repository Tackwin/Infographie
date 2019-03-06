#include "Texture.hpp"

#include "imgui/imgui.h"

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
}
