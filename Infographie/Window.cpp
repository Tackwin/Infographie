#include "Window.hpp"
#include "Scene/Camera.hpp"

details::Window_Struct* details::Window_Struct::instance = nullptr;
details::Window_Struct& Window_Struct = *details::Window_Struct::instance;
