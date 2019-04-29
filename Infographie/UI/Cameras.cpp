#include "Cameras.hpp"

#include "imgui/imgui.h"

#include "Window.hpp"

struct Camera_Config {
	Rectangle2f viewport;
	std::string name;
};
std::vector<Camera_Config> get_viewports_config_for(size_t n) noexcept;

constexpr size_t Max_Cameras = 4;

void update_camera_settings(Camera_Settings& settings) noexcept {
	if (settings.camera_ids.empty()) return;

	thread_local Uuid_t Cam_Selected{ Uuid_t::zero() };
	constexpr auto Temp_Buff_Max = 256;
	thread_local char Temp_Buff[Temp_Buff_Max];

	if (settings.camera_ids.size() < Max_Cameras) {
		if (ImGui::Button("Add Camera")) {
			auto camera_model = (Camera*)settings.root->find_child(settings.camera_ids[0]);
			assert(camera_model);

			auto& new_camera = *settings.root->make_child<Camera>();
			new_camera.render_from((Widget3*)settings.root);
			settings.camera_ids.push_back(new_camera.get_uuid());

			auto cam_configs = get_viewports_config_for(settings.camera_ids.size());

			for (size_t i = 0; i < settings.camera_ids.size(); ++i) {
				auto camera = (Camera*)settings.root->find_child(settings.camera_ids[i]);
				auto config = cam_configs[i];

				assert(camera);

				camera->set_viewport(config.viewport);
				camera->set_name(config.name);
				camera->set_perspective(
					70 / (float)RAD_2_DEG,
					Window_Info.size.x / (float)Window_Info.size.y,
					500.f,
					0.01f
				);
				camera->set_global_position({ 0, 0, -10 });
				camera->look_at({ 0, 0, 0 }, { 0, 1, 0 });
			}
		}
	}

	for (size_t i = 0; i < settings.camera_ids.size(); ++i) {
		auto current_cam = (Camera*)settings.root->find_child(settings.camera_ids[i]);
		ImGui::PushID(current_cam);
		defer{ ImGui::PopID(); };

		bool selected = Cam_Selected == settings.camera_ids[i];

		ImGui::Selectable(current_cam->get_name().c_str(), &selected);

		Cam_Selected = selected ? settings.camera_ids[i] : Cam_Selected;
	}
	for (auto& x : settings.camera_ids) {
		((Camera*)settings.root->find_child(x))->set_input_active(x == Cam_Selected);
	}

	if (Cam_Selected == Uuid_t::zero()) return;

	ImGui::Separator();

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
	auto& current_cam = *((Camera*)settings.root->find_child(Cam_Selected));

	auto exposure = current_cam.get_exposure();
	auto gamma = current_cam.get_gamma();

	switch (Current_Projection) {
	case 0: {
		thread_local float Cam_Fov = 90;
		thread_local float Cam_Far = 500;
		thread_local float Cam_Near = 0.01f;

		ImGui::DragFloat("Fov", &Cam_Fov, 1, 0, 180);
		ImGui::DragFloat("Near", &Cam_Near, 0.001f, 0, 10);
		ImGui::DragFloat("Far", &Cam_Far, 0.001f, 0, 10);

		current_cam.set_perspective(
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

	ImGui::DragFloat("Exposue", &exposure, 0.01f, 0, 1);
	ImGui::DragFloat("Gamma", &gamma, 0.01f, 0, 5);
	ImGui::DragFloat("speed", &current_cam.speed, 0.25f, 0, 200);

	current_cam.set_gamma(gamma);
	current_cam.set_exposure(exposure);
}

std::vector<Camera_Config> get_viewports_config_for(size_t n) noexcept {
	assert(0 < n && n <= Max_Cameras);
	switch (n) {
	case 1: {
		Camera_Config a;
		a.name = "Alfa";
		a.viewport = { 0, 0, 1, 1 };
		return { a };
	}
	case 2: {
		Camera_Config a;
		a.name = "Alpha";
		a.viewport = { 0, 0, 0.5f, 1 };

		Camera_Config b;
		b.name = "Bravo";
		b.viewport = { 0.5f, 0, 0.5f, 1 };
		return { a, b };
	}
	case 3: {
		Camera_Config a;
		a.name = "Alpha";
		a.viewport = { 0, 0, 0.5f, 1 };

		Camera_Config b;
		b.name = "Bravo";
		b.viewport = { 0.5f, 0, 0.5f, 0.5f };

		Camera_Config c;
		c.name = "Charlie";
		c.viewport = { 0.5f, 0.5f, 0.5f, 0.5f };
		return { a, b, c };
	}
	case 4: {
		Camera_Config a;
		a.name = "Alpha";
		a.viewport = { 0, 0, 0.5f, 0.5f };

		Camera_Config b;
		b.name = "Bravo";
		b.viewport = { 0.0f, 0.5f, 0.5f, 0.5f };

		Camera_Config c;
		c.name = "Charlie";
		c.viewport = { 0.5f, 0.0f, 0.5f, 0.5f };

		Camera_Config d;
		d.name = "Delta";
		d.viewport = { 0.5f, 0.5f, 0.5f, 0.5f };
		return { a, b, c, d };
	}
	default: {
		assert(false);
	}
	}
	return {};
}
