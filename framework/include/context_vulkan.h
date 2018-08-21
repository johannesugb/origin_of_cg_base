#pragma once
#ifdef USE_VULKAN_CONTEXT
#include "context_generic_glfw.h"

namespace cgb
{
	struct texture_handle
	{
		int m_vulkan_specific_handle;
	};

	class vulkan : public generic_glfw
	{
	};
}

#endif
