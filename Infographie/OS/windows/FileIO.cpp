#include "OS/FileIO.hpp"
#include "Common.hpp"

std::optional<std::vector<char>>
read_whole_file(const std::filesystem::path& path) noexcept {
	size_t file_length{ 0 };
	FILE* f;
	auto err = fopen_s(&f, path.generic_string().c_str(), "rb");
	if (!f || err) return std::nullopt;

	defer{ fclose(f); };

	fseek(f, 0, SEEK_END);
	file_length = ftell(f);
	rewind(f);
	std::vector<char> bytes(file_length);
	size_t read = fread(bytes.data(), sizeof(char), file_length, f);
	if (read != file_length) return std::nullopt;

	return std::move(bytes);
}

size_t overwrite_file(const std::filesystem::path& path, std::string_view str) noexcept {
	FILE* f;
	auto err = fopen_s(&f, path.generic_string().c_str(), "wb");
	if (!f || err) return err;

	defer{ fclose(f); };

	auto wrote = fwrite(str.data(), 1, str.size(), f);
	if (wrote != str.size()) return EIO;

	return 0;
}