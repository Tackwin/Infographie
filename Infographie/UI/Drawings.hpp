#pragma once
#include <vector>
#include <variant>
#include <functional>
#include "Math/Vector.hpp"
#include "Utils/UUID.hpp"

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

	struct PT_Point {
		Vector4f color{ 0, 0, 0, 1 };
	};
	struct PT_Circle {
		size_t size;
		Vector4f color{ 0, 0, 0, 1 };
		size_t thick;
		Vector4f outline_color;
	};
	struct PT_Rect {
		Vector2u size;
		Vector4f color{ 0, 0, 0, 1 };
		size_t thick;
		Vector4f outline_color;
	};
	struct PT_Polygon {
		size_t n;
		Vector2u size;
		Vector4f color{ 0, 0, 0, 1 };
		size_t thick;
		Vector4f outline_color;
	};
	struct PT_Line {
		size_t thick;
		Vector4f color;
	};

	Widget* root{ nullptr };
	
	bool primitive_tools_selected{ false };
	bool drawing_tools_selected{ false };

	std::vector<Uuid_t> canvases_widget_id;
	std::variant<DT_Circle, DT_Fill, DT_Line, DT_Square> drawing_tool;
	std::variant<PT_Point, PT_Circle, PT_Rect, PT_Polygon, PT_Line> primitive_tool;
	std::vector<std::function<void(Vector2u)>> add_canvas_callback;

	enum class DT_Kind {
		Circle,
		Fill,
		Square,
		Line,
		Count
	};
};

extern void update_drawing_settings(Drawing_Settings& sett) noexcept;

