#pragma once
#if defined(USE_VULKAN_CONTEXT) || defined(USE_OPENGL46_CONTEXT)
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace cgb
{
	/** Context-specific handle to a window.
	 *  For this context, the window is handled via GLFW.
	 */
	struct window_handle
	{
		GLFWwindow* mWindowHandle;
	};

	/** Context-specific handle to a monitor.
	 *  For this context, the monitor is handled via GLFW.
	 */
	struct monitor_handle
	{
		GLFWmonitor* mMonitorHandle;
	};

	// ---------------- forward declarations ----------------
	class window;

	/** \brief Provides generic GLFW-specific functionality
	 */
	class generic_glfw 
	{
	public:
		/** Initializes GLFW */
		generic_glfw();
		
		/** Cleans up GLFW stuff */
		virtual ~generic_glfw();
		
		/** Evaluates to true if GLFW initialization succeeded  */
		operator bool() const;
		
		/** Creates a new window 
		 *  TODO: pass parameters (size, etc.)
		 */
		window create_window();

		/** Close the given window, cleanup the resources */
		void close_window(window& wnd);

		/** Enable or disable v-sync
		 *  This might work only once! :O See GLFW docu for further details!
		 *  http://www.glfw.org/docs/3.1/group__context.html#ga6d4e0cdf151b5e579bd67f13202994ed
		 *
		 *  \param enable true to enable, false to disable v-sync
		 */
		void enable_vsync(bool enable);

	protected:
		static void glfw_error_callback(int error, const char* description);

		bool mInitialized;
		GLFWwindow* mFirstWindow;
	};
}

#endif
