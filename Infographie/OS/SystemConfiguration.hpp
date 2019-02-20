#pragma once
#include <optional>

// in ms
extern std::optional<size_t> get_double_click_time() noexcept;
extern std::optional<size_t> get_key_start_repeat_time() noexcept;
extern std::optional<size_t> get_key_speed_repeat_time() noexcept;
