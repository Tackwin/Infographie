#include "Images.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Scene/Image.hpp"

#include "Utils/Logs.hpp"

#include "Os/OpenFile.hpp"

#include "Managers/AssetsManager.hpp"
#include "Window.hpp"

Vector3f rgb_to_hsv(Vector3i rgb) noexcept;
Vector3i hsv_to_rgb(Vector3f hsv) noexcept;

void update_image_settings(Images_Settings& settings) noexcept {
	constexpr auto Max_Buffer_Size = 2048;
	thread_local char buffer[Max_Buffer_Size];
	std::lock_guard guard{ settings.mutex };

	if (ImGui::Button("Import image")) {
		Open_File_Opts opts;
		opts.allow_redirect_link = false;
		opts.allow_multiple = false;
		opts.dialog_title = "Infographie, Import";

		auto c = push_cursor(sf::Cursor::Wait);
		open_file_async([&, c](Open_File_Result result) {
			pop_cursor(c);
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
		auto c = push_cursor(sf::Cursor::Wait);
		open_dir_async([&, c](std::optional<std::filesystem::path> dir) {
			pop_cursor(c);
			if (!dir) return;
			std::lock_guard guard{ settings.mutex };
			settings.screenshot_directory = *dir;
		});
	}

	ImGui::Separator();
	ImGui::Text("Color space");
	thread_local Vector3i Color_RGB;
	thread_local Vector3f Color_HSV;
	if (ImGui::DragInt3("In rgb", &Color_RGB.x, 1, 0, 255)) {
		Color_HSV = rgb_to_hsv(Color_RGB);
		Color_HSV.x /= 360.f;
	}
	if (ImGui::DragFloat3("In hsv", &Color_HSV.x, 1 / 255.f, 0, 1)) {
		Color_HSV.x *= 360.f;
		Color_RGB = hsv_to_rgb(Color_HSV);
		Color_HSV.x /= 360.f;
	}
	ImGui::SameLine();
	ImGui::Text("?");
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip(
R"_(The h component is normally displayed in the range [0, 360],
here it's normalized)_"
		);
	}

	ImVec4 imgui_color = {
		Color_RGB.x / 255.f,
		Color_RGB.y / 255.f,
		Color_RGB.z / 255.f,
		1
	};
	ImGui::TextColored(imgui_color, "See :D");
	ImGui::Separator();

	if (!settings.root) return;

	ImGui::Columns(2);
	for (size_t i = settings.images_widget_id.size() - 1; i + 1 > 0; --i) {
		ImGui::PushID("Images");
		ImGui::PushID(i);
		auto& img_widget_id = settings.images_widget_id[i];
		if (auto img_widget = (Image*)settings.root->find_child(img_widget_id); img_widget) {
			ImGui::Text("%u", img_widget->get_n());
			ImGui::NextColumn();
			if (ImGui::Button(img_widget->is_open() ? "Close" : "Open")) {
				img_widget->set_open(!img_widget->is_open());
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
			if (img_widget->get_echantillon() && img_widget->size_ok_for_sampling()) {
				possibles_samples_image.push_back(img_widget);
			}
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

	for (size_t y = 0; y < (size_t)Layout_Height; ++y) {
		for (size_t x = 0; x < (size_t)Layout_Width; ++x) {
			ImGui::PushID("Layout");
			ImGui::PushID(x + y * Layout_Width);
			defer{
				ImGui::PopID();
				ImGui::PopID();
			};

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
			if (x + 1 != (size_t)Layout_Width) ImGui::SameLine();
		}
	}

	thread_local Vector2i Image_Size{ 1, 1 };

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
				ech_end.x = Unit_Sample_Size + ech.pos.x;
				ech_end.y = Unit_Sample_Size + ech.pos.y;
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


Vector3f rgb_to_hsv(Vector3i rgb_uint) noexcept {
	Vector3f rgb = ((Vector3f)rgb_uint) / 255.f;
	Vector3f out;
	float min, max, delta;

	min = rgb.x < rgb.y ? rgb.x : rgb.y;
	min = min < rgb.z ? min : rgb.z;

	max = rgb.x > rgb.y ? rgb.x : rgb.y;
	max = max > rgb.z ? max : rgb.z;

	out.z = max;
	delta = max - min;
	if (delta < 0.00001f) {
		out.y = 0;
		out.x = 0; // undefined, maybe nan?
		return out;
	}
	if (max > 0.f) {
		out.y = (delta / max);
	}
	else {
		out.y = 0.f;
		out.x = NAN;
		return out;
	}
	if (rgb.x >= max) {
		out.x = (rgb.y - rgb.z) / delta;
	}
	else {
		if (rgb.y >= max) out.x = 2.f + (rgb.z - rgb.x) / delta;
		else out.x = 4.f + (rgb.x - rgb.y) / delta;
	}

	out.x *= 60.f;
	if (out.x < 0.f) out.x += 360.f;

	return out;
}

Vector3i hsv_to_rgb(Vector3f hsv) noexcept {
	float hh, p, q, t, ff;
	int i;
	Vector3f out;

	hh = hsv.x;
	if (hh >= 360.f) hh = 0.f;
	hh /= 60.f;
	i = (long)hh;
	ff = hh - i;
	p = hsv.z * (1.f - hsv.y);
	q = hsv.z * (1.f - (hsv.y * ff));
	t = hsv.z * (1.f - (hsv.y * (1.f - ff)));

	switch (i) {
	case 0:
		out.x = hsv.z;
		out.y = t;
		out.z = p;
		break;
	case 1:
		out.x = q;
		out.y = hsv.z;
		out.z = p;
		break;
	case 2:
		out.x = p;
		out.y = hsv.z;
		out.z = t;
		break;
	case 3:
		out.x = p;
		out.y = q;
		out.z = hsv.z;
		break;
	case 4:
		out.x = t;
		out.y = p;
		out.z = hsv.z;
		break;
	default:
		out.x = hsv.z;
		out.y = p;
		out.z = q;
		break;
	}

	return (Vector3i)(out * 255);
}