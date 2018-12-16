#pragma once

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
		GLFWwindow* mHandle;
	};

	/** Context-specific handle to a monitor.
	 *  For this context, the monitor is handled via GLFW.
	 */
	struct monitor_handle
	{
		GLFWmonitor* mHandle;
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
	};

	/** Parameters about a framebuffer to be used.
	 */
	struct framebuffer_params
	{
		/** Set to true to enable/request a sRGB compatible framebuffer
		 *	By default (i.e. if not set), it will not be enabled.
		 */
		std::optional<bool> mSrgbFormat;

		/** Set the number of samples for an MSAA-enabled framebuffer.
		 *	By default (i.e. if not set), the number of samples is set to 1,
		 *	which means that MSAA is disabled.
		 */
		std::optional<int> mNumberOfMsaaSamples;

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

	/** Parameters for the swap chain
	 */
	struct swap_chain_params
	{
		/** Parameters about the back buffer(s)
		 */
		framebuffer_params mFramebufferParams;

		/** How to present the images in the back buffer on the surface.
		 *	The default mode is context-specific. Immediate mode will not
		 *	be chosen as the default but instead a more advanced one.
		 */
		std::optional<presentation_mode> mPresentationMode;
	};
}
