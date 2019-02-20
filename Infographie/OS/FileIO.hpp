#pragma once
#include <filesystem>
#include <optional>
#include <vector>

[[nodiscard]] extern std::optional<std::vector<char>>
read_whole_file(const std::filesystem::path& path) noexcept;

[[nodiscard]] extern size_t
overwrite_file(const std::filesystem::path& path, std::string_view str) noexcept;