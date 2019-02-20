#pragma once

#include <thread>
#include <optional>
#include <functional>
#include <filesystem>
#include <unordered_map>

struct Open_File_Opts {
	void* owner{ nullptr };

	std::unordered_map<std::string, std::vector<std::string>> ext_filters;
	std::filesystem::path filepath;
	std::filesystem::path filename;
	std::string dialog_title{ NULL };
	std::string default_ext{ NULL };

	bool allow_multiple{ false };
	bool prompt_for_create{ false };
	bool allow_redirect_link{ false };
};
struct Open_File_Result {
	bool succeded{ false };

	unsigned long error_code;

	std::filesystem::path filepath;
	std::filesystem::path filename;
};

extern void open_dir_async(
	std::function<void(std::optional<std::filesystem::path>)>&& callback
) noexcept;
extern std::optional<std::filesystem::path> open_dir() noexcept;

extern void open_file_async(
	std::function<void(Open_File_Result)>&& callback, Open_File_Opts opts = Open_File_Opts{}
) noexcept;
extern Open_File_Result open_file(Open_File_Opts opts = Open_File_Opts{}) noexcept;
