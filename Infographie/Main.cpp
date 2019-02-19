#include <SFML/Graphics.hpp>
#include "Window.hpp"

int main() {
	details::Window_Struct::instance = new details::Window_Struct();
	defer{ delete details::Window_Struct::instance; };

	Window_Info.title = "Infographie";
	Window_Info.size = { 900, 600 };
	Window_Info.window.create(sf::VideoMode{ UNROLL_2(Window_Info.size) }, Window_Info.title);

	while (Window_Info.window.isOpen()) {
		sf::Event event;
		while (Window_Info.window.pollEvent(event)) {
			if (sf::Event::Closed == event.type) {
				Window_Info.window.close();
			}
		}

		Window_Info.window.clear();
		Window_Info.window.display();
	}
}
