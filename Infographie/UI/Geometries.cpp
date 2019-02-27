#include "Geometries.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Window.hpp"

#include "OS/OpenFile.hpp"
#include "Files/FileFormat.hpp"
#include "Scene/Model.hpp"

#include "Managers/AssetsManager.hpp"

void update_geometries_settings(Geometries_Settings& settings) noexcept {
	std::lock_guard{ settings.mutex };
	sf::ContextSettings context_settings = Window_Info.window.getSettings();

	if (ImGui::CollapsingHeader("Opengl context")) {
		ImGui::Text("Major version: %u", Window_Info.window.getSettings().majorVersion);
		ImGui::Text("Minor version: %u", Window_Info.window.getSettings().minorVersion);
		ImGui::Text("Antialiasing level: %u", Window_Info.window.getSettings().antialiasingLevel);
		ImGui::Text("Depth bits: %u", Window_Info.window.getSettings().depthBits);
		ImGui::Text("Stencil bits: %u", Window_Info.window.getSettings().stencilBits);
	}

	if (ImGui::Button("Load model")) {
		open_file_async([&](Open_File_Result result) {
			std::lock_guard{ settings.mutex };
			if (!result.succeded) return;

			for (auto& f : settings.model_added_callback) {
				f(result.filepath);
			}
		});
	}

	if (ImGui::ImageButton(AM->get_texture("Cube_Icon"), { 20, 20 }, 2)) {
		for (auto& f : settings.spawn_object_callback) {
			f(Object_File::cube({ 1, 1, 1 }));
		}
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(AM->get_texture("Tetra_Icon"), { 20, 20 }, 2)) {
		for (auto& f : settings.spawn_object_callback) {
			f(Object_File::tetraedre({ 1, 1, 1 }));
		}
	}

	if (settings.root) {
		static std::unordered_map<int, bool> selected_map;
		for (auto& m : settings.models_widget_id) {
			ImGui::Columns(5);
			auto model = (Model*)settings.root->find_child(m);
			ImGui::PushID(model);
			defer{ ImGui::PopID(); };

			selected_map[model->get_n()] = model->is_focus();
			ImGui::Checkbox("X", &selected_map[model->get_n()]);
			//model->set_focus(selected_map[model->get_n()]);
			ImGui::NextColumn();
			ImGui::Text("%u", model->get_n());
			ImGui::NextColumn();
			if (!model->get_texture()) {
				if (ImGui::Button("Add texture")) {
					open_file_async([&, m](Open_File_Result result) {
						std::lock_guard{ settings.mutex };
						if (!result.succeded) return;

						for (auto& f : settings.texture_added_callback) {
							f(m, result.filepath);
						}
					});
				}
			}
			ImGui::NextColumn();
			
			if (ImGui::Button("Bounding box")) {
				model->set_render_checkbox(!model->does_render_checkbox());
			}

			ImGui::NextColumn();
			if (ImGui::Button(model->is_visible() ? "Close" : "Open")) {
				model->set_visible(!model->is_visible());
			}
			ImGui::NextColumn();
			ImGui::Columns(1);
		}
	}

}
