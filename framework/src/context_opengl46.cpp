#include "context_generic_glfw.h"
#ifdef USE_OPENGL46_CONTEXT

namespace cgb
{
	opengl46::opengl46() : 
		generic_glfw(), 
		generic_opengl()
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

}

#endif // USE_OPENGL46_CONTEXT
