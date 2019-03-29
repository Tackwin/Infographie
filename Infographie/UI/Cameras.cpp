#include "Cameras.hpp"

#include "imgui/imgui.h"

#include "Window.hpp"

void update_camera_settings(Camera_Settings& settings) noexcept {
	if (settings.camera_ids.empty()) return;

	thread_local Uuid_t Cam_Selected{ Uuid_t::zero() };
	constexpr auto Temp_Buff_Max = 256;
	thread_local char Temp_Buff[Temp_Buff_Max];

	ImGui::Columns(2);
	for (size_t i = 0; i < settings.camera_ids.size(); ++i) {
		auto str = std::to_string(i);
		auto current_cam = (Camera*)settings.root->find_child(settings.camera_ids[i]);
		bool selected =
			Cam_Selected == settings.camera_ids[i] ||
			current_cam->is_input_active();

		strcpy_s(Temp_Buff, current_cam->get_name().c_str());
		if (ImGui::InputText("", Temp_Buff, Temp_Buff_Max)) {
			current_cam->set_name(std::string(Temp_Buff));
		}
		ImGui::NextColumn();
		ImGui::Selectable(Temp_Buff, &selected, ImGuiSelectableFlags_SpanAllColumns, { 0, 0 });

		
		Cam_Selected = selected ? settings.camera_ids[i] : Cam_Selected;
		current_cam->set_input_active(selected);
	}
	ImGui::Columns(1);

	if (Cam_Selected == Uuid_t::zero()) return;

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

	thread_local int Current_Projection{ 0 };
	thread_local const char* Projection_List[]{ "Perspective", "Orthographic" };
	ImGui::ListBox("Projection", &Current_Projection, Projection_List, 2);

	switch (Current_Projection) {
	case 0: {
		thread_local float Cam_Fov = 90;
		thread_local float Cam_Far = 500;
		thread_local float Cam_Near = 1;

		ImGui::DragFloat("Camera fov", &Cam_Fov, 1, 0, 180);
		ImGui::DragFloat("Camera near", &Cam_Near, 0.001f, 0, 10);
		ImGui::DragFloat("Camera far", &Cam_Far, 0.001f, 0, 10);

		((Camera*)settings.root->find_child(Cam_Selected))->set_perspective(
			Cam_Fov / (float)RAD_2_DEG,
			(float)Window_Info.size.x / (float)Window_Info.size.y,
			Cam_Far,
			Cam_Near
		);
		break;
	}
	case 1: {
		thread_local float Cam_Scale = 1;
		thread_local float Cam_Far = 500;
		thread_local float Cam_Near = 1;

		ImGui::DragFloat("Camera scale", &Cam_Scale, 0.01f, 0, 10);
		ImGui::DragFloat("Camera near", &Cam_Near, 0.001f, 0, 10);
		ImGui::DragFloat("Camera far", &Cam_Far, 0.001f, 0, 10);

		((Camera*)settings.root->find_child(Cam_Selected))->set_orthographic(
			Cam_Scale,
			(float)Window_Info.size.x / (float)Window_Info.size.y,
			Cam_Far,
			Cam_Near
		);
		break;
	}
	}
}
