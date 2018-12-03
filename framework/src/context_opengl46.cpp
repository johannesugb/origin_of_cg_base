#include "context_generic_glfw.h"
#ifdef USE_OPENGL46_CONTEXT

namespace cgb
{
	opengl46::opengl46() : generic_glfw()
	{
	}

	window opengl46::create_window()
	{
		// ======= GLFW: Creating a window and context
		glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE); // TODO: make configurable
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE); // TODO: make configurable
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 1); // TODO: make configurable
#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
		return generic_glfw::create_window();
	}

	bool opengl46::check_error(const char* file, int line)
	{
		bool hasError = false;
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			LOG_ERROR("glError int[%d] hex[0x%x] in file[%s], line[%d]", err, err, file, line);
			hasError = true;
		}
		return hasError;
	}

}

#endif // USE_OPENGL46_CONTEXT
