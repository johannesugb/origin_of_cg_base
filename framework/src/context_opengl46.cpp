#include "context_opengl46.h"

namespace cgb
{
	opengl46::opengl46() : generic_glfw()
	{
	}

	window* opengl46::create_window(const window_params& pWndParams, const swap_chain_params& pSwapParams)
	{
		// ======= GLFW: Creating a window and context
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

		// Before creating the window, set some config-parameters
		if (pSwapParams.mFramebufferParams.mSrgbFormat) {
			glfwWindowHint(GLFW_SRGB_CAPABLE, *pSwapParams.mFramebufferParams.mSrgbFormat ? GLFW_TRUE : GLFW_FALSE);
		}
		
		auto presMode = pSwapParams.mPresentationMode ? *pSwapParams.mPresentationMode : presentation_mode::double_buffering;
		switch (presMode) {
		case cgb::presentation_mode::double_buffering:
		case cgb::presentation_mode::vsync:
			glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
			break;
		default:
			break;
		}

		if (pSwapParams.mFramebufferParams.mNumberOfMsaaSamples) {
			glfwWindowHint(GLFW_SAMPLES, *pSwapParams.mFramebufferParams.mNumberOfMsaaSamples);
		}

		auto wnd = generic_glfw::create_window(pWndParams, pSwapParams);
		if (0u == wnd->id() && wnd->handle()) // Only do this for the first window:
		{
			// If context has been newly created in the current call to create_window, 
			// 1) make the newly created context current and
			// 2) use the extension loader to get the proc-addresses (which needs an active context)
			glfwMakeContextCurrent(wnd->handle()->mHandle);
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		}

		switch (presMode) {
		case cgb::presentation_mode::vsync:
			glfwSwapInterval(1);
			break;
		default:
			break;
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
