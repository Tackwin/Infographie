#pragma once
#include <vector>
#include <variant>
#include <functional>
#include "Math/Vector.hpp"
#include "Utils/UUID.hpp"
#include "Graphic/ArrowShape.hpp"

class Widget;
struct Drawing_Settings {
	struct DT_Circle {
		size_t size;
		Vector4f color{ 0, 0, 0, 1 };
	};
	struct DT_Fill {
		Vector4f color{ 0, 0, 0, 1 };
		float tolerance{ 0 };
	};
	struct DT_Square {
		size_t size;
		Vector4f color{ 0, 0, 0, 1 };
	};
	struct DT_Line {
		size_t thick;
		Vector4f color{ 0, 0, 0, 1 };
		bool strip;
	};

	struct PT_Rect {
		Vector2f size{ 100, 50 };
		Vector4f color{ 0, 0, 0, 1 };
		Vector4f outline_color{ 1, 1, 1, 1 };
		float thick{ 0 };
	};
	struct PT_Circle {
		float radius{ 10 };
		Vector4f color{ 0, 0, 0, 1 };
		float thick{ 10 };
		Vector4f outline_color{ 1, 1, 1, 1 };
	};
	struct PT_Polygon {
		size_t n{ 3 };
		float radius{ 5 };
		Vector4f color{ 0, 0, 0, 1 };
		float thick{ 0 };
		Vector4f outline_color{ 1, 1, 1, 1 };
	};
	struct PT_Arrow {
		float thick{ 5 };
		float theta{ 0.f };
		float length{ 10 };
		float outline_thick{ 0 };
		Vector4f color{ 0, 0, 0, 1 };
		Vector4f outline_color{ 1, 1, 1, 1 };
	};

	Widget* root{ nullptr };
	
	bool primitive_tools_selected{ false };
	bool drawing_tools_selected{ false };

	std::vector<Uuid_t> canvases_widget_id;
	std::vector<Uuid_t> primitives_widget_id;

	std::variant<DT_Circle, DT_Fill, DT_Line, DT_Square> drawing_tool;
	std::variant<PT_Rect, PT_Circle, PT_Arrow, PT_Polygon> primitive_tool;
	std::vector<std::function<void(Vector2u)>> add_canvas_callback;
	std::vector<std::function<void(std::unique_ptr<sf::Shape>)>> add_primitive_callback;
	std::vector<std::function<void(std::unique_ptr<Drawable_Transform>)>>
		add_complex_primitive_callback;
};

extern void update_drawing_settings(Drawing_Settings& sett) noexcept;

