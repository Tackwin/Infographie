#include "Common.hpp"
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include "Managers/AssetsManager.hpp"

#include "Utils/Logs.hpp"
#include <Windows.h>

#include "Math/Matrix.hpp"

std::unordered_map<std::string, std::any> Common::debug_values;
Assets_Manager* Common::AM{ nullptr };
bool Common::Is_In_Sfml_Context{ false };
Matrix4f* Common::View_Matrix{ nullptr };
Matrix4f* Common::Projection_Matrix{ nullptr };
float Common::Alpha_Tolerance{ 0.f };

void Common::check_gl_error() {
	if (auto err = glGetError(); err != GL_NO_ERROR) {
		// So that was a nice day ruined...
		if (!Is_In_Sfml_Context) Log.push(
			"You are tring to do the next error outside of the opengl thread is that cool ?"
		);
		std::string str((const char* const)glewGetErrorString(err));
		DebugBreak();
		Log.push(str);
	}
}

// courtesy of
// https://stackoverflow.com/questions/18064988/rendering-issue-with-different-computers/18067245#18067245

const char* debug_source_str(GLenum source) {
	static const char* sources[] = {
	  "API",   "Window System", "Shader Compiler", "Third Party", "Application",
	  "Other", "Unknown"
	};
	int str_idx = std::min(
		source - GL_DEBUG_SOURCE_API, sizeof(sources) / sizeof(const char*) - 1
	);
	return sources[str_idx];
}

const char* debug_type_str(GLenum type) {
	static const char* types[] = {
	  "Error",       "Deprecated Behavior", "Undefined Behavior", "Portability",
	  "Performance", "Other",               "Unknown"
	};

	int str_idx = std::min(type - GL_DEBUG_TYPE_ERROR, sizeof(types) / sizeof(const char*) - 1);
	return types[str_idx];
}

const char* debug_severity_str(GLenum severity) {
	static const char* severities[] = {
	  "High", "Medium", "Low", "Unknown"
	};

	int str_idx = std::min(
		severity - GL_DEBUG_SEVERITY_HIGH, sizeof(severities) / sizeof(const char*) - 1
	);
	return severities[str_idx];
}

void GLAPIENTRY Common::verbose_opengl_error(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei,
	const char* message,
	GLvoid*
) noexcept {
	//if (severity != GL_DEBUG_SEVERITY_HIGH) return;

	printf("OpenGL Error:\n");
	printf("=============\n");
	printf(" Object ID: ");
	printf("%d\n", id);
	printf(" Severity:  ");
	printf(debug_severity_str(severity));
	printf(" Type:      ");
	printf("%s\n", debug_type_str(type));
	printf(" Source:    ");
	printf("%s\n", debug_source_str(source));
	printf(" Message:   ");
	printf("%s\n\n", message);
	fflush(stdout);
	//DebugBreak();
}