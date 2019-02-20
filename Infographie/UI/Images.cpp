#include "Images.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Scene/Image.hpp"

#include <Os/OpenFile.hpp>

void render_images_settings(Images_Settings& settings) noexcept {
	std::lock_guard{ settings.mutex };

	if (settings.closed) return;

	ImGui::Begin("Images", &settings.closed);
	defer{ ImGui::End(); };

	if (ImGui::Button("Import image")) {
		Open_File_Opts opts;
		opts.allow_redirect_link = false;
		opts.allow_multiple = false;
		opts.dialog_title = "Infographie, Import";

		open_file_async([&](Open_File_Result result) {
			std::lock_guard{ settings.mutex };
			if (!result.succeded) return;

			for (auto& x : settings.import_images_callback) x(result.filepath);
		}, opts);
	}

	if (!settings.root) return;

	ImGui::Columns(2);
	for (size_t i = settings.images_widget_id.size() - 1; i + 1 > 0; --i) {
		auto& img_widget_id = settings.images_widget_id[i];
		if (auto img_widget = (Image*)settings.root->find_child(img_widget_id); img_widget) {
			ImGui::Text("%u", img_widget->n);
			ImGui::NextColumn();
			if (ImGui::Button("Open")) {
				img_widget->open = true;
			}
			ImGui::NextColumn();
		}
		else {
			settings.images_widget_id.erase(std::begin(settings.images_widget_id) + i);
		}
	}
}

