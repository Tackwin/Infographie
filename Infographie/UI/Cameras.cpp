#include "Cameras.hpp"

#include "imgui/imgui.h"

void update_camera_settings(Camera_Settings& settings) noexcept {
	if (settings.camera_ids.empty()) return;

	thread_local Uuid_t Cam_Selected{ Uuid_t::zero() };
	for (size_t i = 0; i < settings.camera_ids.size(); ++i) {
		auto str = std::to_string(i);

		bool selected = Cam_Selected == settings.camera_ids[i];
		ImGui::Selectable(str.c_str(), &selected);
		Cam_Selected = selected ? settings.camera_ids[i] : Cam_Selected;
	}

	if (ImGui::Button("Track selected")) {
		std::vector<Uuid_t> selected;

		settings.root->for_every_childs([&selected](Widget* w) {
			if (auto w3 = dynamic_cast<Widget3*>(w); w3 && w3->is_focus()) {
				selected.push_back(w3->get_uuid());
			}
		});

		((Camera*)settings.root->find_child(Cam_Selected))->lock(selected);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset track")) {
		((Camera*)settings.root->find_child(Cam_Selected))->lock({});
	}
}
