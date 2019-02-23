#include "Drawings.hpp"
#include <type_traits>
#include <variant>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Scene/Canvas.hpp"

#include "Managers/AssetsManager.hpp"
constexpr auto Image_Button_Border = 1;

void update_drawing_tools(Drawing_Settings& settings) noexcept;

void update_drawing_settings(Drawing_Settings& settings) noexcept {

	static int New_Canvas_Size[2]{ 300, 300 };
	ImGui::InputInt2("Size", New_Canvas_Size);
	ImGui::SameLine();
	if (ImGui::Button("Add Canvas")) {
		for (auto& f : settings.add_canvas_callback)
			f( {(size_t)New_Canvas_Size[0], (size_t)New_Canvas_Size[1] });
	}

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

	if (!settings.root) return;

	ImGui::Separator();
	ImGui::Columns(2);
	for (size_t i = settings.canvases_widget_id.size() - 1; i + 1 > 0; --i) {
		auto& img_widget_id = settings.canvases_widget_id[i];
		if (auto canvas_widget = (Canvas*)settings.root->find_child(img_widget_id); canvas_widget)
		{
			ImGui::Text("%u", canvas_widget->get_n());
			ImGui::NextColumn();
			if (ImGui::Button("Open")) {
				canvas_widget->set_visible(true);
			}
			ImGui::NextColumn();
		}
		else {
			settings.canvases_widget_id.erase(std::begin(settings.canvases_widget_id) + i);
		}
	}
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
			ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&x.color));

			x.size = (size_t)Circle_Size_Int;
		}

		if constexpr (std::is_same_v<T, Drawing_Settings::DT_Fill>) {
			ImGui::Text("Fill parameters");

			ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&x.color));
			ImGui::InputFloat("Tolerance", reinterpret_cast<float*>(&x.tolerance));
		}

		if constexpr (std::is_same_v<T, Drawing_Settings::DT_Square>) {
			static int Square_Size_Int{ 0 };
			ImGui::Text("Square parameters");

			ImGui::InputInt("Size", &Square_Size_Int, 1, 5);
			ImGui::ColorPicker3("Color", reinterpret_cast<float*>(&x.color));
			x.size = (size_t)Square_Size_Int;
		}

		if constexpr (std::is_same_v<T, Drawing_Settings::DT_Line>) {
			static int Line_Thick_Int{ 0 };
			ImGui::Text("Line parameters");

			ImGui::InputInt("Thickness", &Line_Thick_Int, 1, 5);
			ImGui::ColorPicker4("Color", reinterpret_cast<float*>(&x.color));
			ImGui::Checkbox("Strip", &x.strip);
		}
	}, settings.drawing_tool);

}

