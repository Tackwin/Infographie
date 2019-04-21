#include "Topologie.hpp"

#include "imgui/imgui.h"

#include "Scene/Bezier.hpp"

void update_topologie_settings(Topologie_Settings& settings) noexcept {
	ImGui::PushItemWidth(100);

	static int points{ 5 };
	ImGui::InputInt("Points :", &points);
	points = std::max(2, points);

	ImGui::PopItemWidth();

	ImGui::SameLine();

	if (ImGui::Button("Add Curve")) for (auto& x : settings.added_curve) x(points);

	ImGui::PushItemWidth(100);

	static int points_surface{ 5 };
	ImGui::InputInt("Points² :", &points_surface);
	points = std::max(2, points_surface);

	ImGui::PopItemWidth();

	ImGui::SameLine();

	if (ImGui::Button("Add Surface")) for (auto& x : settings.added_surface) x(points_surface);

	ImGui::Separator();
	if (!settings.root) return;

	settings.root->for_every_childs([](Widget * w) {
		if (auto b = dynamic_cast<Bezier*>(w); b) {
			ImGui::PushID(b);
			ImGui::Text("Bezier curve %u", b->get_n_points());
			ImGui::SameLine();

			bool is_open = b->is_open();
			ImGui::Checkbox("", &is_open);
			b->set_open(is_open);

			ImGui::PopID();
		}
	});
}
