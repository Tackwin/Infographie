#include "Geometries.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Window.hpp"

#include "OS/OpenFile.hpp"
#include "Scene/Model.hpp"

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

	if (settings.root) {

		static std::unordered_map<int, bool> render_checkbox_map;
		for (auto& m : settings.models_widget_id) {
			auto model = (Model*)settings.root->find_child(m);
			ImGui::BeginGroup();
			defer{ ImGui::EndGroup(); };
			ImGui::Columns(4);
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
