#include <SFML/Graphics.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "Window.hpp"

#include "UI/Images.hpp"
#include "UI/Drawings.hpp"
#include "Scene/Widget.hpp"
#include "Scene/Image.hpp"
#include "Scene/Canvas.hpp"
#include "Managers/AssetsManager.hpp"
#include "Managers/InputsManager.hpp"
#include "Utils/TimeInfo.hpp"

void load_textures() noexcept;

void construct_managers() noexcept;
void destroy_managers() noexcept;

void update(
	Widget& root, Images_Settings& img_settings, Drawing_Settings& draw_settings, float dt
) noexcept;
void render(Widget& root, sf::RenderTarget& target) noexcept;

int main() {
	construct_managers();
	defer{ destroy_managers(); };

	load_textures();

	details::Window_Struct::instance = new details::Window_Struct();
	defer{ delete details::Window_Struct::instance; };

	Window_Info.title = "Infographie";
	Window_Info.size = { 1600u, 900u };
	Window_Info.window.create(sf::VideoMode{ UNROLL_2(Window_Info.size) }, Window_Info.title);

	ImGui::SFML::Init(Window_Info.window);

	Images_Settings img_settings;
	Drawing_Settings draw_settings;

	Widget root;
	img_settings.root = &root;
	draw_settings.root = &root;

	img_settings.import_images_callback.push_back([&](const std::filesystem::path& path) {
		if (!AM->load_texture(path.generic_string(), path)) return;
		auto img_widget = root.make_child<Image>(AM->get_texture(path.generic_string()));
		img_settings.images_widget_id.push_back(img_widget->get_uuid());
	});

	draw_settings.add_canvas_callback.push_back([&](Vector2u size) {
		auto canvas_widget = root.make_child<Canvas>(draw_settings);
		canvas_widget->set_size(size);
		draw_settings.canvases_widget_id.push_back(canvas_widget->get_uuid());
	});

	sf::Clock dt_clock;
	float dt;
	while (Window_Info.window.isOpen()) {
		dt = dt_clock.restart().asSeconds();
		IM::update(Window_Info.window);

		update(root, img_settings, draw_settings, dt);

		Window_Info.window.clear();
		render(root, Window_Info.window);
		Window_Info.window.display();
	}
	ImGui::SFML::Shutdown();
}

void update(
	Widget& root, Images_Settings& img_settings, Drawing_Settings& draw_settings, float dt
) noexcept {
	ImGui::SFML::Update(Window_Info.window, sf::seconds(dt));
	root.propagate_input();


	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove);
	defer{ ImGui::End(); };

	ImGui::SetWindowPos({ 5, 5 });

	if (ImGui::CollapsingHeader("Image")) update_image_settings(img_settings);
	if (ImGui::CollapsingHeader("Drawing")) update_drawing_settings(draw_settings);



	root.propagate_update(dt);

	if (img_settings.take_screenshot) {
		defer{ img_settings.take_screenshot = false; };

		sf::RenderTexture render_texture;
		render_texture.create(UNROLL_2(Window_Info.size));
		render_texture.clear();
		render(root, render_texture);
		render_texture.display();

		auto number_of_files = std::filesystem::hard_link_count(img_settings.screenshot_directory);
		auto file_name =
			img_settings.screenshot_directory /
			("screenshot_" + std::to_string(number_of_files) + ".png");

		render_texture.getTexture().copyToImage().saveToFile(file_name.generic_string());
	}
}

void render(Widget& root, sf::RenderTarget& target) noexcept {
	ImGui::SFML::Render(target);
	root.propagate_render(target);
}


void construct_managers() noexcept {
	AM = new Assets_Manager();
}

void destroy_managers() noexcept {
	delete AM;
}

void load_textures() noexcept {
	AM->load_texture("Primitives_Tool", "res/Primitives_Tool.png");
	AM->load_texture("Drawings_Tool", "res/Drawings_Tool.png");
	AM->load_texture("DT_Circle", "res/DT_Circle.png");
	AM->load_texture("DT_Line", "res/DT_Line.png");
	AM->load_texture("DT_Square", "res/DT_Square.png");
	AM->load_texture("DT_Fill", "res/DT_Fill.png");
}
