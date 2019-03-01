#pragma once
#include "vulkan_context.h"

namespace cgb {
	class vulkan_memory
	{
	public:
		vulkan_memory();
		~vulkan_memory();

		vk::DeviceMemory memory;
		vk::DeviceSize offset;
	};

}