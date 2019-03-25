#include "Illumination.hpp"

#include "imgui/imgui.h"

#include "Scene/Model.hpp"
#include "Scene/LightPoint.hpp"

void update_illumination_settings(Illumination_Settings& settings) noexcept {

	ImGui::DragFloat("Ambient strength", &settings.ambient.strength, 1, 0, 1);
	ImGui::ColorPicker3("Ambient color", &settings.ambient.color.r);

	ImGui::Separator();
	if (ImGui::Button("+")) {
		auto w = settings.root->make_child<Light_Point>();
		settings.light_point_ids.push_back(w->get_uuid());
	}

	ImGui::Separator();
	ImGui::PushItemWidth(100);
	for (auto& id : settings.light_point_ids) {
		auto x = (Light_Point*)settings.root->find_child(id);

		auto color = x->get_light_color();

		ImGui::PushID(x);
		ImGui::ColorPicker3("Color", &color.x);
		ImGui::PopID();

		x->set_light_color(color);
	}
	ImGui::PopItemWidth();


}
