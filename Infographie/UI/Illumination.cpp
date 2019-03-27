#include "Illumination.hpp"

#include "imgui/imgui.h"

#include "Scene/Model.hpp"
#include "Scene/LightPoint.hpp"

void update_illumination_settings(Illumination_Settings& settings) noexcept {

	ImGui::DragFloat("Ambient strength", &settings.ambient.strength, 0.01f, 0, 1);
	ImGui::ColorPicker3("Ambient color", &settings.ambient.color.x);

	ImGui::Separator();
	if (ImGui::Button("Add point")) {
		auto w = settings.root->make_child<Light_Point>();
		settings.light_point_ids.push_back(w->get_uuid());
	}
	if (ImGui::Button("Add directional")) {
		settings.directionals.emplace_back();
	}

	ImGui::Separator();
	ImGui::PushItemWidth(100);
	for (auto& id : settings.light_point_ids) {
		auto x = (Light_Point*)settings.root->find_child(id);

		auto color = x->get_light_color();
		auto strength = x->get_strength();

		ImGui::PushID(x);
		ImGui::ColorPicker3("Color", &color.x);
		ImGui::DragFloat("Strength", &strength, 0.01f, 0);
		ImGui::PopID();

		x->set_light_color(color);
		x->set_strength(strength);
	}
	ImGui::PopItemWidth();

	ImGui::Separator();

	ImGui::PushItemWidth(100);
	for (size_t i = 0; i < settings.directionals.size(); ++i) {
		auto& x = settings.directionals[i];

		Vector2f dir2{ x.dir.x, x.dir.y };
		Vector2f angles;
		angles.x = std::acosf(x.dir.x / dir2.length()) * (x.dir.y < 0 ? -1 : 1);
		angles.y = std::acosf(x.dir.z / x.dir.length());

		ImGui::PushID(i);

		ImGui::ColorPicker3("Color", &x.color.x);
		ImGui::DragFloat2("Dir", &angles.x, 0.001f);
		ImGui::DragFloat("Strength", &x.strength, 0.01f, 0);

		x.dir.x = std::sinf(angles.y) * std::cosf(angles.x);
		x.dir.y = std::sinf(angles.y) * std::sinf(angles.x);
		x.dir.z = std::cosf(angles.y);
		
		ImGui::PopID();
	}
	ImGui::PopItemWidth();

}
