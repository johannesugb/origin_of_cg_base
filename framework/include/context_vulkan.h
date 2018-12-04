#ifdef USE_VULKAN_CONTEXT
#pragma once

// DEFINES:
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// INCLUDES:
#include "context_generic_glfw.h"
#include "context_vulkan_types.h"

namespace cgb
{
	/**	@brief Context for Vulkan
	 *
	 *	This context abstracts calls to the Vulkan API, for environment-related
	 *	stuff, like window creation etc.,  it relies on GLFW and inherits
	 *	@ref generic_glfw.
	 */
	class vulkan : public generic_glfw
	{
	public:
		vulkan();

		window create_window(const window_params&);

		texture_handle create_texture()
		{
			return texture_handle();
		}

		void destroy_texture(const texture_handle& pHandle)
		{

		}
	};
}

#endif // USE_VULKAN_CONTEXT
