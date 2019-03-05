#include "Transform.hpp"

#include <string>
#include <unordered_map>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Scene/Model.hpp"

#include "Managers/InputsManager.hpp"

void update_transform_settings(Transform_Settings& settings) noexcept {

	std::vector<Model*> focused_models;
	
	static std::unordered_map<Model*, Vector3<Vector3f>> transform_snapshot;
	
	settings.root->for_every_childs([&](Widget* x) {
		if (auto w3 = dynamic_cast<Model*>(x); w3 && w3->is_focus())
			focused_models.push_back(w3);
	});

	ImGui::Text("Selected models: ");
	ImGui::Columns(5);
	for (auto& x : focused_models) {
		if (auto m = dynamic_cast<Model*>(x)) {
			if (!transform_snapshot.count(x)) {
				transform_snapshot[m] = {
					m->get_global_position3(),
					m->get_rotation3(),
					m->get_scaling()
				};
			}

			ImGui::Text("%u", m->get_n());
			ImGui::NextColumn();
		}
	}
	ImGui::Columns(1);

	static Vector3f Translate{ 0.f, 0.f, 0.f };
	static Vector3f Rotation{ 0, 0, 0 };
	static Vector3f Scaling{ 0, 0, 0 };

	if (ImGui::DragFloat3("Translate", &Translate.x, 0.001f, -5, 5, "%.3f", 3)) {
		for (auto& x : focused_models) {
			x->set_global_position(x->get_global_position3() + Translate);
		}
	}
	if (ImGui::DragFloat3("Rotation", &Rotation.x, 0.1f, -5, 5)) {
		for (auto& x : focused_models) {
			x->set_rotation(x->get_rotation3() + (Rotation / RAD_2_DEG));
		}
	}
	if (ImGui::DragFloat3("Scaling", &Scaling.x, 0.1f, -1, 1)) {
		for (auto& x : focused_models) {
			x->set_scaling(x->get_scaling() + (Scaling));
		}
	}

	if ((
			Rotation != Vector3f{ 0, 0, 0} ||
			Scaling != Vector3f{0, 0, 0} ||
			Translate != Vector3f{0, 0, 0}
		) &&
		!IM::isMousePressed(sf::Mouse::Left)
	) {
		// we get the uuids because Models might be destroyed by the time we go for the undo
		// so with uuid we could check if they still exist where as with raw pointer
		// we would segfault, or worse UB :(
		std::vector<Uuid_t> uuid_focused;
		for (auto& x : focused_models) uuid_focused.push_back(x->get_uuid());

		std::unordered_map<Uuid_t, Vector3<Vector3f>> transform_diff;

		for (auto& x : focused_models) {
			Vector3<Vector3f> diff = {
				x->get_global_position3() - transform_snapshot[x].x,
				x->get_rotation3() - transform_snapshot[x].y,
				x->get_scaling() - transform_snapshot[x].z
			};
			transform_diff[x->get_uuid()] = diff;
		}

		// we need to clear the redos because with the "diff scheme" we have no garantee that
		// inserting an action in the middle of the todo list won't mess everything up.
		// besides i don't thnik it's a feature so important that we need to make up a
		// solution to this problem.
		settings.redos.clear();
		settings.undos.push_back({
			[settings, uuid_focused, d = transform_diff] {
				for (auto i : uuid_focused) {
					if (auto m = (Model*)settings.root->find_child(i); m) {
						m->set_global_position(m->get_global_position3() - d.at(i).x);
						m->set_rotation(m->get_rotation3() - d.at(i).y);
						m->set_scaling(m->get_scaling() - d.at(i).z);
					}
				}
			},
			[settings, uuid_focused, d = transform_diff] {
				for (auto i : uuid_focused) {
					if (auto m = (Model*)settings.root->find_child(i); m) {
						m->set_global_position(m->get_global_position3() + d.at(i).x);
						m->set_rotation(m->get_rotation3() + d.at(i).y);
						m->set_scaling(m->get_scaling() + d.at(i).z);
					}
				}
			},
		});
		
		Rotation = { 0, 0, 0 };
		Scaling = { 0, 0, 0 };
		Translate = { 0, 0, 0 };
		transform_snapshot.clear();
	}

	ImGui::Columns(2);
	ImGui::BeginGroup();
	ImGui::BeginChild("Undos", { 0, 180 }, true);

	// Undos are a heap so we start end first
	for (size_t i = settings.undos.size() - 1; i + 1 > 0; --i) {
		ImGui::PushID(i);
		defer{ ImGui::PopID(); };

		if (ImGui::Button("Undo")) {
			for (size_t j = settings.undos.size() - 1; j + 1  > i; --j) {
				settings.undos[j].undo();
				transform_snapshot.clear();
				settings.redos.push_back(settings.undos[j]);
				settings.undos.pop_back();
			}
			continue;
		}
	}

	ImGui::EndChild();
	ImGui::EndGroup();
	ImGui::NextColumn();
	ImGui::BeginGroup();
	ImGui::BeginChild("Redos", { 0, 180 }, true);
	// Redos are the sames as Undos
	for (size_t i = settings.redos.size() - 1; i + 1 > 0; --i) {
		ImGui::PushID(i);
		defer{ ImGui::PopID(); };

		if (ImGui::Button("Redo")) {
			for (size_t j = settings.redos.size() - 1; j + 1 > i; --j) {
				settings.redos[j].redo();
				transform_snapshot.clear();
				settings.undos.push_back(settings.redos[j]);
				settings.redos.pop_back();
			}
			continue;
		}
	}
	ImGui::EndChild();
	ImGui::EndGroup();
	ImGui::Columns(1);

	if (ImGui::Button("Clear History")) {
		settings.undos.clear();
		settings.redos.clear();
	}
}
