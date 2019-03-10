#include <GL/glew.h>
#include "Geometries.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Window.hpp"

#include "OS/OpenFile.hpp"
#include "Files/FileFormat.hpp"
#include "Scene/Model.hpp"

#include "Managers/AssetsManager.hpp"
#include "Managers/InputsManager.hpp"

#include "Window.hpp"


void update_geometries_settings(Geometries_Settings& settings) noexcept {
	std::lock_guard out_guard{ settings.mutex };
	sf::ContextSettings context_settings = Window_Info.window.getSettings();
	ImGui::Spacing(); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
	if (ImGui::CollapsingHeader("Opengl context")) {
		ImGui::Text("Major version: %u", Window_Info.window.getSettings().majorVersion);
		ImGui::Text("Minor version: %u", Window_Info.window.getSettings().minorVersion);
		ImGui::Text("Antialiasing level: %u", Window_Info.window.getSettings().antialiasingLevel);
		ImGui::Text("Depth bits: %u", Window_Info.window.getSettings().depthBits);
		ImGui::Text("Stencil bits: %u", Window_Info.window.getSettings().stencilBits);
		int max_texture{ 0 };
		glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_texture);
		ImGui::Text("Max active texture: %u", max_texture);
	}

	if (ImGui::Button("Load model")) {
		auto c = push_cursor(sf::Cursor::Wait);
		open_file_async([&, c](Open_File_Result result) {
			pop_cursor(c);
			std::lock_guard guard{ settings.mutex };
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
		thread_local std::unordered_map<Model*, bool> selected_map;
		for (auto& m : settings.models_widget_id) {
			ImGui::Columns(4);
			auto model = (Model*)settings.root->find_child(m);
			ImGui::PushID(model);
			defer{ ImGui::PopID(); };
			if (ImGui::Checkbox("Is focused", &selected_map[model])) {
				model->set_focus(selected_map[model]);
				model->lock_focus(selected_map[model]);
			}
			if (model->is_focus() != selected_map[model]) {
				selected_map[model] = model->is_focus();
			}
			ImGui::NextColumn();
			ImGui::Text("%u", model->get_n());
			ImGui::NextColumn();
			if (ImGui::Button("Main Texture")) {
				auto c = push_cursor(sf::Cursor::Wait);
				open_file_async([&, m, c](Open_File_Result result) {
					pop_cursor(c);
					std::lock_guard guard{ settings.mutex };
					if (!result.succeded) return;

					for (auto& f : settings.texture_callback) {
						f(m, result.filepath, Geometries_Settings::Texture_Type::Normal);
					}
				});
			}
			ImGui::NextColumn();
			if (ImGui::Button("Gradient Texture")) {
				for (auto& f : settings.texture_generated_set_callback) f(m);
			}
			ImGui::NextColumn();

			// We went over the line (that is 4 columns wide)
			// We skip the first column of the new line to align with the last.
			ImGui::NextColumn();

			if (ImGui::Button("Alpha Texture")) {
				auto c = push_cursor(sf::Cursor::Wait);
				open_file_async([&, m, c](Open_File_Result result) {
					pop_cursor(c);
					std::lock_guard guard{ settings.mutex };
					if (!result.succeded) return;

					for (auto& f : settings.texture_callback) {
						f(m, result.filepath, Geometries_Settings::Texture_Type::Alpha);
					}
				});
			}
			ImGui::NextColumn();

			if (ImGui::Button("Box")) {
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
