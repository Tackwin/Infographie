#include "Images.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Scene/Image.hpp"

#include "Utils/Logs.hpp"

#include "Os/OpenFile.hpp"

#include "Managers/AssetsManager.hpp"

void update_image_settings(Images_Settings& settings) noexcept {
	constexpr auto Max_Buffer_Size = 2048;
	thread_local char buffer[Max_Buffer_Size];
	std::lock_guard guard{ settings.mutex };

	if (ImGui::Button("Import image")) {
		Open_File_Opts opts;
		opts.allow_redirect_link = false;
		opts.allow_multiple = false;
		opts.dialog_title = "Infographie, Import";

		open_file_async([&](Open_File_Result result) {
			std::lock_guard guard{ settings.mutex };
			if (!result.succeded) return;

			for (auto& x : settings.import_images_callback) x(result.filepath);
		}, opts);
	}

	if (ImGui::Button("Screenshot")) {
		if (settings.screenshot_directory.empty()) {
			Log.push("Please select screenshot directory");
			Log.show = true;
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
			std::lock_guard guard{ settings.mutex };
			settings.screenshot_directory = *dir;
		});
	}

	if (!settings.root) return;

	ImGui::Columns(2);
	for (size_t i = settings.images_widget_id.size() - 1; i + 1 > 0; --i) {
		ImGui::PushID("Images");
		ImGui::PushID(i);
		auto& img_widget_id = settings.images_widget_id[i];
		if (auto img_widget = (Image*)settings.root->find_child(img_widget_id); img_widget) {
			ImGui::Text("%u", img_widget->get_n());
			ImGui::NextColumn();
			if (ImGui::Button("Open")) {
				img_widget->set_open(true);
			}
			ImGui::NextColumn();
		}
		else {
			settings.images_widget_id.erase(std::begin(settings.images_widget_id) + i);
		}
		ImGui::PopID();
		ImGui::PopID();
	}
	ImGui::Columns(1);

	ImGui::Separator();

	static constexpr auto Unit_Sample_Size = 30u;
	thread_local auto Layout_Width = 1;
	thread_local auto Layout_Height= 1;
	thread_local Vector3f Color_Filler{ 0.1f, 0.2f, 0.3f };
	thread_local std::unordered_map<Vector2u, size_t> Layout_Sample;
	
	std::vector<Image*> possibles_samples_image;
	for (auto& img_id : settings.images_widget_id) {
		if (auto img_widget = (Image*)settings.root->find_child(img_id); img_widget) {
			if (img_widget->get_echantillon()) possibles_samples_image.push_back(img_widget);
		}
	}

	ImGui::Text("Create from echantillons");
	ImGui::PushItemWidth(100);
	ImGui::ColorPicker3("Filler", &Color_Filler.x);
	ImGui::PopItemWidth();
	ImGui::Columns(3);
	ImGui::Text("Layout");
	ImGui::NextColumn();
	ImGui::DragInt("Width", &Layout_Width, 1, 1, 5);
	ImGui::NextColumn();
	ImGui::DragInt("Height", &Layout_Height, 1, 1, 5);
	ImGui::NextColumn();
	ImGui::Columns(1);

	for (size_t x = 0; x < (size_t)Layout_Width; ++x) {
		for (size_t y = 0; y < (size_t)Layout_Height; ++y) {
			auto s = Layout_Sample[{x, y}];

			sf::Sprite sprite;
			if ((s % (1 + possibles_samples_image.size())) == 0) {
				sprite.setTexture(AM->get_texture("White"));
				sprite.setColor({
					(sf::Uint8)(255 * Color_Filler.x),
					(sf::Uint8)(255 * Color_Filler.y),
					(sf::Uint8)(255 * Color_Filler.z),
					255
					});
			}
			else {
				s = (s - 1) % possibles_samples_image.size();
				sprite.setColor({ 255, 255, 255, 255 });
				sprite = *possibles_samples_image[s]->get_echantillon_sprite();
			}

			auto clicked = ImGui::ImageButton(
				sprite,
				{ (float)Unit_Sample_Size, (float)Unit_Sample_Size },
				-1,
				sf::Color::Transparent,
				sprite.getColor()
			);
			if (clicked) {
				++Layout_Sample[{x, y}];
			}
			if (y + 1 != (size_t)Layout_Height) ImGui::SameLine();
		}
	}

	thread_local Vector2i Image_Size;

	ImGui::Columns(3);
	ImGui::Text("Image size(in samples)");
	ImGui::NextColumn();
	ImGui::DragInt("Width##size", &Image_Size.x, 1, 1, 20);
	ImGui::NextColumn();
	ImGui::DragInt("Height##size", &Image_Size.y, 1, 1, 20);
	ImGui::NextColumn();
	ImGui::Columns(1);
	if (ImGui::Button("Create from samples")) {
		sf::Image created;
		created.create(Image_Size.x* Unit_Sample_Size, Image_Size.y* Unit_Sample_Size, {
			(sf::Uint8)(255 * Color_Filler.x),
			(sf::Uint8)(255 * Color_Filler.y),
			(sf::Uint8)(255 * Color_Filler.z),
			255
		});

		for (size_t x = 0; x < (size_t)Image_Size.x; ++x) {
			for (size_t y = 0; y < (size_t)Image_Size.y; ++y) {
				auto sample = Layout_Sample[{x % Layout_Width, y % Layout_Height}];

				// the default color is already the Color_Filler virtue of 11 line ago.
				if ((sample % (1 + possibles_samples_image.size())) == 0) continue;
				
				sample = (sample - 1) % possibles_samples_image.size();
				auto ech = *possibles_samples_image[sample]->get_echantillon();

				Vector2u ech_end;
				ech_end.x = std::min(ech.size.x, Unit_Sample_Size) + ech.pos.x;
				ech_end.y = std::min(ech.size.y, Unit_Sample_Size) + ech.pos.y;
				for (size_t a = ech.pos.x; a < ech_end.x; ++a) {
					for (size_t b = ech.pos.y; b < ech_end.y; ++b) {
						auto color = ech.pixels->getPixel(a, b);
						created.setPixel(
							x * Unit_Sample_Size + a - ech.pos.x,
							y * Unit_Sample_Size + b - ech.pos.y,
							color
						);
					}
				}
			}
		}

		for (auto& f : settings.create_images_callback) {
			f(created);
		}
	}

}

