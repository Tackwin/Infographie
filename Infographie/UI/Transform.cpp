#include "Transform.hpp"

#include <unordered_map>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Scene/Model.hpp"

#include "Managers/InputsManager.hpp"

void update_transform_settings(const Transform_Settings& settings) noexcept {
	static std::unordered_map<Model*, Vector3f> model_snapshot_pos;
	std::vector<Model*> focused_models;
	settings.root->for_every_childs([&](Widget* x) {
		if (auto w3 = dynamic_cast<Model*>(x); w3 && w3->is_focus())
			focused_models.push_back(w3);
	});

	ImGui::Text("Selected models: ");
	ImGui::Columns(5);
	for (auto& x : focused_models) {
		if (auto m = dynamic_cast<Model*>(x)) {
			ImGui::Text("%u", m->get_n());
			ImGui::NextColumn();
		}
	}
	ImGui::Columns(1);

	static Vector3f Translate{ 0.f, 0.f, 0.f };
	static bool Is_Translating{ false };
	ImGui::SliderFloat("Translate X", &Translate.x, -100, 100, "%.3f", 2);
	ImGui::SliderFloat("Translate Y", &Translate.y, -100, 100, "%.3f", 2);
	ImGui::SliderFloat("Translate Z", &Translate.z, -100, 100, "%.3f", 2);

	if (!Is_Translating && IM::isMouseJustPressed(sf::Mouse::Left)) {
		Is_Translating = true;
		model_snapshot_pos.clear();
		for (auto& x : focused_models) model_snapshot_pos[x] = x->get_global_position3();
	}
	if (Is_Translating && !IM::isMousePressed(sf::Mouse::Left)) {
		Is_Translating = false;
		Translate = { 0, 0, 0 };
	}

	if (Is_Translating) {
		for (auto& [x, t] : model_snapshot_pos) {
			x->set_global_position(t + Translate);
		}
	}
}
