#include "Window.hpp"
#include "Scene/Camera.hpp"

details::Window_Struct* details::Window_Struct::instance = nullptr;
details::Window_Struct& Window_Struct = *details::Window_Struct::instance;

sf::Cursor* push_cursor(sf::Cursor::Type type) noexcept {
	std::lock_guard guard{ Window_Info.cursor_mutex };
	auto ptr = std::make_unique<sf::Cursor>();
	ptr->loadFromSystem(type);
	auto raw_ptr = ptr.get();
	Window_Info.window.setMouseCursor(*ptr);
	Window_Info.cursor_stack.push_back(std::move(ptr));
	return raw_ptr;
}

void pop_cursor(sf::Cursor* cursor) noexcept {
	std::lock_guard guard{ Window_Info.cursor_mutex };
	// we won't pop the last cursor
	assert(Window_Info.cursor_stack.size() > 1);

	for (size_t i = Window_Info.cursor_stack.size() - 1; i + 1 > 1; --i) {
		if (Window_Info.cursor_stack[i].get() == cursor) {
			Window_Info.cursor_stack.erase(std::begin(Window_Info.cursor_stack) + i);
			Window_Info.window.setMouseCursor(*Window_Info.cursor_stack[i - 1]);
			return;
		}
	}
	assert(false);
}


