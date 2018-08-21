#if defined(USE_VULKAN_CONTEXT) || defined(USE_OPENGL46_CONTEXT)
#include "context_generic_glfw.h"

namespace cgb
{
	generic_glfw::generic_glfw() : 
		mInitialized(false),
		mFirstWindow(nullptr)
	{
		LOG_VERBOSE("Creating GLFW context...");
	
		// Setting an error callback
		glfwSetErrorCallback(glfw_error_callback);
		
		// Initializing GLFW
		if (GLFW_TRUE == glfwInit())
		{
			mInitialized = true;
		}
		else
		{
			LOG_ERROR("glfwInit failed");
		}
	}

	generic_glfw::~generic_glfw()
	{
		if (mInitialized)
		{
			glfwTerminate();
			// context has been desroyed by glfwTerminate
			mFirstWindow = nullptr;
			mInitialized = false;
		}
	}

	generic_glfw::operator bool() const
	{
		return mInitialized;
	}

	window generic_glfw::create_window()
	{
		// For the first window, let GLFW create a new context...
		GLFWwindow* contextToUse = nullptr; 
		if (mFirstWindow)
		{
			// ...for all further windows, share the context
			contextToUse = mFirstWindow; 

			// TODO: what happens to the context and to the other windows, if the first window is closed?
		}

		GLFWwindow* handle = glfwCreateWindow(
			800, // TODO: make width configurable
			600, // TODO: make height configurable
			"TODO: make configurable",
			nullptr, // TODO: make Monitor configurable
			contextToUse); // TODO: make configurable
		
		if (!handle)
		{
			throw std::runtime_error("glfwCreateWindow failed"); 
		}

		if (!mFirstWindow)
		{
			// If context has been newly created in the current call to create_window, 
			// 1) make the newly created context current and
			// 2) use the extension loader to get the proc-addresses (which needs an active context)
			glfwMakeContextCurrent(handle);
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
			mFirstWindow = handle; // don't do this again next time
			// By (current) design, all windows share the same context.
			// TODO: Think about supporting different contexts somewhen in the future.
		}

		return window(window_handle{ handle });
	}

	void generic_glfw::close_window(window& wnd)
	{
		if (!wnd.handle())
		{
			LOG_WARNING("The passed window has no valid handle. Has it already been destroyed?");
			return;
		}

		glfwDestroyWindow(wnd.handle()->mWindowHandle);
		// TODO: set window's handle to std::nullopt
	}

	void generic_glfw::enable_vsync(bool enable)
	{
		glfwSwapInterval(enable ? 1 : 0);
	}

	void generic_glfw::glfw_error_callback(int error, const char* description)
	{
		LOG_ERROR("GLFW-Error: hex[0x%x] int[%d] description[%s]", error, error, description);
	}

}

#endif // defined(USE_VULKAN_CONTEXT) || defined(USE_OPENGL46_CONTEXT)
