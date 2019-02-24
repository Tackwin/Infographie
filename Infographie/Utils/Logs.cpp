#include "Logs.hpp"

#include "imgui/imgui.h"

details::Logs Log;

void details::Logs::push(std::string str) noexcept {
	std::lock_guard{ Log.mutex };
	Log.data.push_back(std::move(str));
}


void list_all_logs_imgui() noexcept {
	std::lock_guard{ Log.mutex };
	if (!Log.show || Log.data.empty()) return;
	ImGui::Begin(
		"Logs",
		&Log.show,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse
	);
	defer{ ImGui::End(); };

	for (size_t i = Log.data.size() - 1; i + 1 > 0; --i) {
		ImGui::Text(Log.data[i].c_str());
		ImGui::SameLine();
		if (ImGui::Button("X")) {
			Log.data.erase(std::begin(Log.data) + i);
		}
	}
}
