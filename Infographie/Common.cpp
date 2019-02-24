#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include "Common.hpp"

#include "Managers/AssetsManager.hpp"

#include "Utils/Logs.hpp"
#include <Windows.h>

#include "Math/Matrix.hpp"

std::unordered_map<std::string, std::any> Common::debug_values;
Assets_Manager* Common::AM{ nullptr };
bool Common::Is_In_Sfml_Context{ false };
Matrix4f* Common::View_Matrix{ nullptr };
Matrix4f* Common::Projection_Matrix{ nullptr };

void Common::check_gl_error() {
	if (auto err = glGetError(); err != GL_NO_ERROR) {
		std::string str((const char* const)glewGetErrorString(err));
		DebugBreak();
		Log.push(str);
	}
}