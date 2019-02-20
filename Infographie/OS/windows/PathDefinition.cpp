#ifdef _WIN32

#include "OS/PathDefinition.hpp"

#include <Windows.h>

#include "Common.hpp"

std::filesystem::path get_executable_path() noexcept {
	char* buffer;
	size_t size{ 128 };
	size_t copied{ 0 };

	while(true) {
		size *= 2;

		buffer = new char[size];

		copied = (size_t)GetModuleFileName(NULL, buffer, size);

		if (copied != 0 || GetLastError() != ERROR_INSUFFICIENT_BUFFER) break;

		delete buffer;
	};
	assert(copied != 0); // That means that we have an unexpected error.

	std::filesystem::path path{ buffer };
	delete buffer;
	return path;
}

std::filesystem::path get_executable_dir() noexcept {
	return get_executable_path().parent_path();
}

std::filesystem::path get_working_dir() noexcept {
	return std::filesystem::current_path();
}

#endif
