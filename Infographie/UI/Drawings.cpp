#include <SFML/Graphics.hpp>

#include "Drawings.hpp"
#include <type_traits>
#include <variant>


#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Scene/Canvas.hpp"
#include "Window.hpp"

#include "Managers/AssetsManager.hpp"
constexpr auto Image_Button_Border = 1;

void update_drawing_tools(Drawing_Settings& settings) noexcept;
void update_primitive_tools(Drawing_Settings& settings) noexcept;

void update_drawing_settings(Drawing_Settings& settings) noexcept {

	static int New_Canvas_Size[2]{ 300, 300 };
	ImGui::InputInt2("Size", New_Canvas_Size);
	ImGui::SameLine();
	if (ImGui::Button("Add Canvas")) {
		for (auto& f : settings.add_canvas_callback)
			f( {(size_t)New_Canvas_Size[0], (size_t)New_Canvas_Size[1] });
	}

	ImGui::PushItemWidth(300);
	ImGui::ColorPicker3("Background Color", reinterpret_cast<float*>(&Window_Info.clear_color));
	ImGui::PopItemWidth();

	auto& pt_texture = AM->get_texture("Primitives_Tool");
	sf::Vector2f pt_texture_size{ UNROLL_2_P(pt_texture.getSize(), float) };
	auto& dt_texture = AM->get_texture("Drawings_Tool");
	sf::Vector2f dt_texture_size{ UNROLL_2_P(dt_texture.getSize(), float) };
	if (ImGui::ImageButton(pt_texture, pt_texture_size, Image_Button_Border)) {
		settings.primitive_tools_selected = !settings.primitive_tools_selected;
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(dt_texture, dt_texture_size, Image_Button_Border)) {
		settings.drawing_tools_selected = !settings.drawing_tools_selected;
	}

	if (settings.drawing_tools_selected) update_drawing_tools(settings);
	if (settings.primitive_tools_selected) update_primitive_tools(settings);

	if (!settings.root) return;

	ImGui::Separator();
	ImGui::Columns(2);
	for (size_t i = settings.canvases_widget_id.size() - 1; i + 1 > 0; --i) {
		auto& img_widget_id = settings.canvases_widget_id[i];
		if (auto canvas_widget = (Canvas*)settings.root->find_child(img_widget_id); canvas_widget)
		{
			ImGui::Text("%u", canvas_widget->get_n());
			ImGui::NextColumn();
			if (ImGui::Button(canvas_widget->is_visible() ? "Close" : "Open")) {
				canvas_widget->set_visible(!canvas_widget->is_visible());
			}
			ImGui::NextColumn();
		}
		else {
			settings.canvases_widget_id.erase(std::begin(settings.canvases_widget_id) + i);
		}
	}
	ImGui::Columns(1);
}

void update_drawing_tools(Drawing_Settings& settings) noexcept {
	ImGui::Separator();
	ImGui::Separator();

	// If onlyyyy there was a feature that programming language had where you could generate
	// your code based on compile time info at compile time...
	// i wonder if that exist somewhere...

	// >SEE 22/02/19 The draft withe paper for Circle a superset of c++17 with full compile time
	// code execution, AST manipulation have been released. No public compiler release for now
	// but if it's half as good as i think i'll be glad.

	auto& dt_circle_texture = AM->get_texture("DT_Circle");
	sf::Vector2f dt_circle_texture_size{ UNROLL_2_P(dt_circle_texture.getSize(), float) };
	auto& dt_square_texture = AM->get_texture("DT_Square");
	sf::Vector2f dt_square_texture_size{ UNROLL_2_P(dt_square_texture.getSize(), float) };
	auto& dt_fill_texture = AM->get_texture("DT_Fill");
	sf::Vector2f dt_fill_texture_size{ UNROLL_2_P(dt_fill_texture.getSize(), float) };
	auto& dt_line_texture = AM->get_texture("DT_Line");
	sf::Vector2f dt_line_texture_size{ UNROLL_2_P(dt_line_texture.getSize(), float) };


	if (ImGui::ImageButton(dt_circle_texture, dt_circle_texture_size, Image_Button_Border)) {
		settings.drawing_tool = Drawing_Settings::DT_Circle{};
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(dt_square_texture, dt_square_texture_size, Image_Button_Border)) {
		settings.drawing_tool = Drawing_Settings::DT_Square{};
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(dt_fill_texture, dt_fill_texture_size, Image_Button_Border)) {
		settings.drawing_tool = Drawing_Settings::DT_Fill{};
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(dt_line_texture, dt_line_texture_size, Image_Button_Border)) {
		settings.drawing_tool = Drawing_Settings::DT_Line{};
	}

	ImGui::Separator();

	std::visit([](auto&& x) {
		using T = std::decay_t<decltype(x)>;

		if constexpr (std::is_same_v<T, Drawing_Settings::DT_Circle>) {
			static int Circle_Size_Int{ 0 };

			ImGui::Text("Circle parameters");
			ImGui::InputInt("Size", &Circle_Size_Int, 1, 5);

			x.size = (size_t)Circle_Size_Int;
		}

		if constexpr (std::is_same_v<T, Drawing_Settings::DT_Fill>) {
			ImGui::Text("Fill parameters");
			ImGui::InputFloat("Tolerance", reinterpret_cast<float*>(&x.tolerance));
		}

		if constexpr (std::is_same_v<T, Drawing_Settings::DT_Square>) {
			static int Square_Size_Int{ 0 };

			ImGui::Text("Square parameters");
			ImGui::InputInt("Size", &Square_Size_Int, 1, 5);

			x.size = (size_t)Square_Size_Int;
		}

		if constexpr (std::is_same_v<T, Drawing_Settings::DT_Line>) {
			static int Line_Thick_Int{ 0 };

			ImGui::Text("Line parameters");
			ImGui::InputInt("Thickness", &Line_Thick_Int, 1, 5);
			ImGui::Checkbox("Strip", &x.strip);
		}
		ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&x.color));

	}, settings.drawing_tool);

}

void update_primitive_tools(Drawing_Settings& settings) noexcept {
	ImGui::Separator();
	ImGui::Separator();

	auto& pt_polygon_texture = AM->get_texture("PT_Polygon");
	sf::Vector2f pt_polygon_texture_size{ UNROLL_2_P(pt_polygon_texture.getSize(), float) };
	auto& pt_arrow_texture = AM->get_texture("PT_Arrow");
	sf::Vector2f pt_arrow_texture_size{ UNROLL_2_P(pt_arrow_texture.getSize(), float) };
	auto& pt_circle_texture = AM->get_texture("DT_Circle");
	sf::Vector2f pt_circle_texture_size{ UNROLL_2_P(pt_circle_texture.getSize(), float) };
	auto& pt_rect_texture = AM->get_texture("PT_Rect");
	sf::Vector2f pt_rect_texture_size{ UNROLL_2_P(pt_rect_texture.getSize(), float) };
	auto& pt_heart_texture = AM->get_texture("PT_Heart");
	sf::Vector2f pt_heart_texture_size{ UNROLL_2_P(pt_heart_texture.getSize(), float) };

	if (ImGui::ImageButton(pt_polygon_texture, pt_polygon_texture_size, Image_Button_Border)) {
		settings.primitive_tool = Drawing_Settings::PT_Polygon{};
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(pt_arrow_texture, pt_arrow_texture_size, Image_Button_Border)) {
		settings.primitive_tool = Drawing_Settings::PT_Arrow{};
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(pt_circle_texture, pt_circle_texture_size, Image_Button_Border)) {
		settings.primitive_tool = Drawing_Settings::PT_Circle{};
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(pt_rect_texture, pt_rect_texture_size, Image_Button_Border)) {
		settings.primitive_tool = Drawing_Settings::PT_Rect{};
	}
	ImGui::SameLine();
	if (ImGui::ImageButton(pt_heart_texture, pt_heart_texture_size, Image_Button_Border)) {
		settings.primitive_tool = Drawing_Settings::PT_Heart{};
	}
	ImGui::Separator();

	std::visit([](auto&& x) {
		using T = std::decay_t<decltype(x)>;
		if constexpr (std::is_same_v<T, Drawing_Settings::PT_Polygon>) {
			// I need to do those thread_local/static variables because i like to be restrictive
			// with my types (uses unsigned where it makes sense etc...)
			// but imgui interface with only int for instance.
			thread_local int Faces{ 3 };
			thread_local float Radius{ 10 };
			thread_local float Outline_Thickness{ 0 };
			thread_local Vector4f Color{ 0, 0, 0, 1 };
			thread_local Vector4f Outline_Color{ 1, 1, 1, 1 };
			
			ImGui::PushID("Polygon");
			ImGui::DragInt("Number of faces", &Faces, 1, 3, 20);
			ImGui::DragFloat("Radius", &Radius, 0.5, 5, 200);
			ImGui::DragFloat("Outline thickness", &Outline_Thickness, 0.5, 0, Radius);
			
			ImGui::PushItemWidth(ImGui::GetWindowWidth() / 3);
			ImGui::ColorPicker4("Fill Color", &Color.x);
			ImGui::SameLine();
			ImGui::ColorPicker4("Outline Color", &Outline_Color.x);
			ImGui::PopItemWidth();
			ImGui::PopID();

			x.n = Faces;
			x.radius = Radius;
			x.thick = Outline_Thickness;
			x.color = Color;
			x.outline_color = Outline_Color;
		}
		if constexpr (std::is_same_v<T, Drawing_Settings::PT_Arrow>) {
			// I need to do those thread_local/static variables because i like to be restrictive
			// with my types (uses unsigned where it makes sense etc...)
			// but imgui interface with only int for instance.
			// here i do a deg to rad conversion
			thread_local float Length{ 3 };
			thread_local float Thickness{ 0 };
			thread_local float Outline_Thickness{ 0 };
			thread_local Vector4f Color{ 0, 0, 0, 1 };
			thread_local Vector4f Outline_Color{ 1, 1, 1, 1 };

			ImGui::PushID("Arrow");
			ImGui::DragFloat("Length", &Length, 1, 10, 200);
			ImGui::DragFloat("Thickness", &Thickness, 1, 5, 200);
			ImGui::DragFloat("Outline thickness", &Outline_Thickness, 1, 0, Thickness);
			ImGui::PushItemWidth(ImGui::GetWindowWidth() / 3);
			ImGui::ColorPicker4("Fill Color", &Color.x);
			ImGui::SameLine();
			ImGui::ColorPicker4("Outline Color", &Outline_Color.x);
			ImGui::PopItemWidth();
			ImGui::PopID();

			x.length = Length;
			x.thick = Thickness;
			x.outline_thick = Outline_Thickness;
			x.color = Color;
			x.outline_color = Outline_Color;
		}
		if constexpr (std::is_same_v<T, Drawing_Settings::PT_Circle>) {
			thread_local float Radius{ 20 };
			thread_local float Thickness{ 0 };
			thread_local Vector4f Color{ 0, 0, 0, 1 };
			thread_local Vector4f Outline_Color{ 1, 1, 1, 1 };

			ImGui::PushID("Circle");
			ImGui::DragFloat("Radius", &Radius, 0.1f, 1, 200);
			ImGui::DragFloat("Thickness", &Thickness, 0.1f, 0, Radius);
			ImGui::PushItemWidth(ImGui::GetWindowWidth() / 3);
			ImGui::ColorPicker4("Fill Color", &Color.x);
			ImGui::SameLine();
			ImGui::ColorPicker4("Outline Color", &Outline_Color.x);
			ImGui::PopItemWidth();
			ImGui::PopID();

			x.radius = Radius;
			x.thick = Thickness;
			x.color = Color;
			x.outline_color = Outline_Color;
		}
		if constexpr (std::is_same_v<T, Drawing_Settings::PT_Rect>) {
			thread_local Vector2f Size{ 100, 50 };
			thread_local float Thickness{ 0 };
			thread_local Vector4f Color{ 0, 0, 0, 1 };
			thread_local Vector4f Outline_Color{ 1, 1, 1, 1 };

			ImGui::PushID("Rect");
			ImGui::DragFloat("Thickness", &Thickness, 0.1f, 0, std::min(Size.x, Size.y) / 2);
			ImGui::DragFloat2("Size", &Size.x, 0.1f, 1, 200);
			ImGui::PushItemWidth(ImGui::GetWindowWidth() / 3);
			ImGui::ColorPicker4("Fill Color", &Color.x);
			ImGui::SameLine();
			ImGui::ColorPicker4("Outline Color", &Outline_Color.x);
			ImGui::PopItemWidth();
			ImGui::PopID();

			x.size = Size;
			x.thick = Thickness;
			x.color = Color;
			x.outline_color = Outline_Color;
		}
		if constexpr (std::is_same_v<T, Drawing_Settings::PT_Heart>) {
			thread_local Vector2f Size{ 100, 50 };
			thread_local float Thickness{ 0 };
			thread_local Vector4f Color{ 0, 0, 0, 1 };
			thread_local Vector4f Outline_Color{ 1, 1, 1, 1 };

			ImGui::PushID("Heart");
			ImGui::DragFloat("Thickness", &Thickness, 0.1f, 0, std::min(Size.x, Size.y) / 2);
			ImGui::DragFloat2("Size", &Size.x, 0.1f, 1, 200);
			ImGui::PushItemWidth(ImGui::GetWindowWidth() / 3);
			ImGui::ColorPicker4("Fill Color", &Color.x);
			ImGui::SameLine();
			ImGui::ColorPicker4("Outline Color", &Outline_Color.x);
			ImGui::PopItemWidth();
			ImGui::PopID();

			x.size = Size;
			x.thick = Thickness;
			x.color = Color;
			x.outline_color = Outline_Color;
		}
	}, settings.primitive_tool);

	if (!ImGui::Button("Add")) return;

	std::visit([&](auto&& x) {
		using T = std::decay_t<decltype(x)>;
		if constexpr (std::is_same_v<T, Drawing_Settings::PT_Polygon>) {
			auto shape = std::make_unique<sf::ConvexShape>();
			shape->setPointCount(x.n);
			shape->setOutlineColor((sf::Color)x.outline_color);
			shape->setFillColor((sf::Color)x.color);
			shape->setOutlineThickness(x.thick);

			// >SEE is this necessary ?
			//shape->setOrigin(a.radius, a.radius);

			for (size_t i = 0; i < x.n; ++i) {
				auto t = 2 * PIf * (float)i / x.n;
				shape->setPoint(i, Vector2f::createUnitVector(t) * x.radius);
			}

			for (auto& f : settings.add_primitive_callback) {
				// copy construct a new entity to move it to the callback.
				auto ptr = std::make_unique<sf::ConvexShape>(*shape);
				f(std::move(ptr));
			}
		}
		if constexpr (std::is_same_v<T, Drawing_Settings::PT_Arrow>) {
			auto shape = std::make_unique<Arrow_Shape>();
			shape->length = x.length;
			shape->thick = x.thick;
			shape->outline_thick = x.outline_thick;
			shape->color = x.color;
			shape->outline_color = x.outline_color;

			for (auto& f : settings.add_complex_primitive_callback) {
				// copy construct a new entity to move it to the callback.
				auto ptr = std::make_unique<Arrow_Shape>(*shape);
				f(std::move(ptr));
			}
		}
		if constexpr (std::is_same_v<T, Drawing_Settings::PT_Circle>) {
			Drawing_Settings::PT_Circle& a = x;

			auto shape = std::make_unique<sf::CircleShape>();
			shape->setOutlineColor((sf::Color)a.outline_color);
			shape->setFillColor((sf::Color)a.color);
			shape->setOutlineThickness(a.thick);
			shape->setOrigin(a.radius / 2, a.radius / 2);

			for (auto& f : settings.add_primitive_callback) {
				// copy construct a new entity to move it to the callback.
				auto ptr = std::make_unique<sf::CircleShape>(*shape);
				f(std::move(ptr));
			}
		}
		if constexpr (std::is_same_v<T, Drawing_Settings::PT_Rect>) {
			auto shape = std::make_unique<sf::RectangleShape>();
			shape->setOutlineColor((sf::Color)x.outline_color);
			shape->setFillColor((sf::Color)x.color);
			shape->setOutlineThickness(x.thick);
			shape->setOrigin(x.size / 2);
			shape->setSize(x.size);

			for (auto& f : settings.add_primitive_callback) {
				// copy construct a new entity to move it to the callback.
				auto ptr = std::make_unique<sf::RectangleShape>(*shape);
				f(std::move(ptr));
			}
		}
		if constexpr (std::is_same_v<T, Drawing_Settings::PT_Heart>) {
			auto shape = std::make_unique<Heart_Shape>();
			shape->outline_color = x.outline_color;
			shape->color = x.color;
			shape->thick = x.thick;
			shape->size = x.size;

			for (auto& f : settings.add_complex_primitive_callback) {
				// copy construct a new entity to move it to the callback.
				auto ptr = std::make_unique<Heart_Shape>(*shape);
				f(std::move(ptr));
			}
		}
	}, settings.primitive_tool);
}
