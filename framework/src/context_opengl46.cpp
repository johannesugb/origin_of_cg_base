#include "context_opengl46.h"

namespace cgb
{
	opengl46::opengl46() : generic_glfw()
	{
	}

	window opengl46::create_window(const window_params& pParams)
	{
		// ======= GLFW: Creating a window and context
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
		auto wnd = generic_glfw::create_window(pParams);
		if (0u == wnd.id() && wnd.handle()) // Only do this for the first window:
		{
			// If context has been newly created in the current call to create_window, 
			// 1) make the newly created context current and
			// 2) use the extension loader to get the proc-addresses (which needs an active context)
			glfwMakeContextCurrent(wnd.handle()->mWindowHandle);
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
			// By (current) design, all windows share the same context.
			// TODO: Think about supporting different contexts somewhen in the future.
		}
		return wnd;
	}

	bool opengl46::check_error(const char* file, int line)
	{
		bool hasError = false;
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			LOG_ERROR(fmt::format("glError int[{:d}] hex[0x{:x}] in file[{}], line[{}]", err, err, file, line));
			hasError = true;
		}
		return hasError;
	}

}
