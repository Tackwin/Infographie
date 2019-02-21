#include "Images.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Scene/Image.hpp"

#include <Os/OpenFile.hpp>

void update_image_settings(Images_Settings& settings) noexcept {
	constexpr auto Max_Buffer_Size = 2048;
	thread_local char buffer[Max_Buffer_Size];
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

	if (ImGui::Button("Screenshot")) {
		if (settings.screenshot_directory.empty()) {
			ImGui::OpenPopup("Alert");
			settings.log.push_back("Please select screenshot directory");
		}
		else {
			settings.take_screenshot = true;
		}
	}
	ImGui::SameLine();
	auto screenshot_button_label = settings.screenshot_directory.empty()
		? std::string{ "Please select screenshot directory" }
			: settings.screenshot_directory.generic_string();
	if (ImGui::Button(screenshot_button_label.c_str())) {
		open_dir_async([&](std::optional<std::filesystem::path> dir) {
			if (!dir) return;
			std::lock_guard{ settings.mutex };
			settings.screenshot_directory = *dir;
		});
	}

	if (ImGui::BeginPopup("Alert")) {
		if (ImGui::Button("Clear")) {
			settings.log.clear();
		}
		for (size_t i = settings.log.size() - 1; i + 1 > 0; --i) {
			ImGui::Text(settings.log[i].c_str());
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				settings.log.erase(std::begin(settings.log) + i);
			}
		}
		if (settings.log.empty()) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
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

