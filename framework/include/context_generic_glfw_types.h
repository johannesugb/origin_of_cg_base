#pragma once

#include <GLFW/glfw3.h>

// GLM:
#define GLM_FORCE_SWIZZLE 
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/mat4x4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/hash.hpp>

namespace cgb
{
	/** Context-specific handle to a window.
	 *  For this context, the window is handled via GLFW.
	 */
	struct window_handle
	{
		GLFWwindow* mHandle;
	};

	/** Context-specific handle to a monitor.
	 *  For this context, the monitor is handled via GLFW.
	 */
	struct monitor_handle
	{
		/** Gets a monitor handle to the primary monitor */
		static monitor_handle primary_monitor()
		{
			return monitor_handle{ glfwGetPrimaryMonitor() };
		}

		/** Gets a monitor handle to ANY secondary monitor */
		static monitor_handle secondary_monitor()
		{
			auto primary = primary_monitor();
			int count;
			GLFWmonitor** monitors = glfwGetMonitors(&count);
			for (int i = 0; i < count; ++i) {
				if (monitors[i] != primary.mHandle) {
					return monitor_handle{ monitors[i] };
				}
			}
			throw std::runtime_error("No secondary monitor found");
		}

		GLFWmonitor* mHandle;
	};

	/** Different options on how to present the images in the back buffer on 
	 *	the surface. 
	 */
	enum struct presentation_mode
	{
		/** Submit images immediately to the screen */
		immediate,
		/** Use two buffers: front-buffer and back-buffer */
		double_buffering,
		/** Double buffering and wait for "vertical blank" */
		vsync,
		/** Triple buffering mode, also called "mailbox" mode */
		triple_buffering
	};

}
