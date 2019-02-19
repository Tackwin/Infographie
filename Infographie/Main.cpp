#include <SFML/Graphics.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "Window.hpp"

int main() {
	details::Window_Struct::instance = new details::Window_Struct();
	defer{ delete details::Window_Struct::instance; };

	Window_Info.title = "Infographie";
	Window_Info.size = { 900, 600 };
	Window_Info.window.create(sf::VideoMode{ UNROLL_2(Window_Info.size) }, Window_Info.title);

	ImGui::SFML::Init(Window_Info.window);

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

		ImGui::Begin("Hello, world!");
		ImGui::Button("Look at this pretty button");
		ImGui::End();
		Window_Info.window.clear();
		ImGui::SFML::Render(Window_Info.window);
		Window_Info.window.display();
	}
	ImGui::SFML::Shutdown();
}
