#pragma once
#if defined(USE_VULKAN_CONTEXT) || defined(USE_OPENGL46_CONTEXT)

#include <GLFW/glfw3.h>

// GLM:
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/mat4x4.hpp>

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

	/** Parameters for creating a GLFW window
	 */
	struct window_params
	{
		/** Desired height of the window */
		std::optional<int> mInitialWidth;
		
		/** Desired width of the window */
		std::optional<int> mInitialHeight;
		
		/** Window title */
		std::string mWindowTitle;
		
		/** Optional monitor handle to assign the window to.
		 *	If this is set, the window will run in fullscreen mode. 
		 */
		std::optional<monitor_handle> mMonitor;
		
		/** Set this to another window to use a shared context */
		std::optional<window_handle> mSharedContext;
		
		/** Set to true to enable/request a sRGB compatible framebuffer 
		 *	By default (i.e. if not set), it will not be enabled.
		 */
		std::optional<bool> mEnableSrgbFramebuffer;
		
		/** Set to true to enable double-buffering.
		 *	By default (i.e. if not set), it will be enabled. 
		 */
		std::optional<bool> mEnableDoublebuffering;

		/** Set the number of samples for an MSAA-enabled framebuffer.
		 *	By default (i.e. if not set), the number of samples is set to 1, 
		 *	which means that MSAA is disabled.
		 */
		std::optional<int> mNumberOfSamplesForMSAA;
	};

}

#endif
