#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

#include "Widget.hpp"
#include "UI/Drawings.hpp"

class Canvas : public Widget {
public:
	static size_t Total_N;
	Canvas(const Drawing_Settings& settings) noexcept;

	void update(float dt) noexcept override;
	void render(sf::RenderTarget& target) noexcept override;

	void set_size(const Vector2f& size) noexcept override;
	void set_size(const Vector2u& size) noexcept;

	void paint_pixels(const std::vector<Vector2u>& pixels, Vector4f color) noexcept;
	sf::Image& get_image() noexcept;

	size_t get_n() const noexcept;
private:

	void fill_circle(Vector2u pos, size_t size, Vector4f color) noexcept;
	void fill_square(Vector2u center, size_t size, Vector4f color) noexcept;
	void fill_line(Vector2u A, Vector2u B, size_t thick, Vector4f color) noexcept;
	void flood_fill(Vector2u pos, Vector4u color, float tolerance) noexcept;

	void action_on_canvas() noexcept;

	const Drawing_Settings& settings;

	size_t n{ 0 };

	bool texture_cached{ false };
	bool mouse_is_in_canvas{ false };
	std::optional<Vector2u> line_start_point;

	sf::Sprite sprite;
	sf::Texture texture;
	sf::RenderTexture render_texture;
	sf::Image image;

	sf::Cursor* click_cursor{ nullptr };
	sf::Cursor* hover_cursor{nullptr};
};
