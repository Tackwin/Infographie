#include <SFML/Graphics.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "Window.hpp"

#include "UI/Images.hpp"
#include "Scene/Widget.hpp"
#include "Scene/Image.hpp"
#include "Managers/AssetsManager.hpp"

void construct_managers() noexcept;
void destroy_managers() noexcept;

int main() {
	construct_managers();
	defer{ destroy_managers(); };
	
	details::Window_Struct::instance = new details::Window_Struct();
	defer{ delete details::Window_Struct::instance; };



	Window_Info.title = "Infographie";
	Window_Info.size = { 1280, 720 };
	Window_Info.window.create(sf::VideoMode{ UNROLL_2(Window_Info.size) }, Window_Info.title);

	ImGui::SFML::Init(Window_Info.window);

	Images_Settings img_settings;
	Widget root;
	img_settings.root = &root;

	img_settings.import_images_callback.push_back([&](const std::filesystem::path& path) {
		if (!AM->load_texture(path.generic_string(), path)) return;
		auto img_widget = root.make_child<Image>(AM->get_texture(path.generic_string()));
		img_settings.images_widget_id.push_back(img_widget->get_uuid());
	});

	sf::Clock dt_clock;
	float dt;
	while (Window_Info.window.isOpen()) {
		dt = dt_clock.restart().asSeconds();
		sf::Event event;
		while (Window_Info.window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);
			if (sf::Event::Closed == event.type) {
				Window_Info.window.close();
			}
		}
		ImGui::SFML::Update(Window_Info.window, sf::seconds(dt));
		root.propagate_input();

		render_images_settings(img_settings);

		root.propagate_update(dt);

		Window_Info.window.clear();
		ImGui::SFML::Render(Window_Info.window);
		root.propagate_render(Window_Info.window);
		Window_Info.window.display();
	}
	ImGui::SFML::Shutdown();
}


void construct_managers() noexcept {
	AM = new Assets_Manager();
}

void destroy_managers() noexcept {
	delete AM;
}
