#pragma once
#ifdef USE_VULKAN_CONTEXT

namespace cgb
{
	struct texture_handle
	{
		int m_vulkan_specific_handle;
	};
}

#endif // USE_VULKAN_CONTEXT
