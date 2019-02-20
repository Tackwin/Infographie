#ifdef _WIN32
#include "OS/SystemConfiguration.hpp"

#include <Windows.h>

std::optional<size_t> get_double_click_time() noexcept {
	return (size_t)GetDoubleClickTime();
}
std::optional<size_t> get_key_repeat_start_time() noexcept {
	DWORD x;
	if (!SystemParametersInfoA(0, SPI_GETKEYBOARDDELAY, &x, 0)) return std::nullopt;
	return (size_t)x;
}
std::optional<size_t> get_key_repeat_speed_time() noexcept {
	DWORD x;
	if (!SystemParametersInfoA(0, SPI_GETKEYBOARDSPEED, &x, 0)) return std::nullopt;
	return (size_t)x;
}
#endif