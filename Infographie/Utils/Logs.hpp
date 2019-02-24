#pragma once
#include <shared_mutex>
#include <vector>
#include <string>

namespace details {
	struct Logs {
		std::shared_mutex mutex;

		std::vector<std::string> data;
		bool show{ false };

		void push(std::string str) noexcept;
	};

}
extern details::Logs Log;

extern void list_all_logs_imgui() noexcept;
