#include "vulkan_memory_manager.h"

#include <stdexcept>

namespace cgb {

	vulkan_memory_manager::vulkan_memory_manager()
	{
	}


	vulkan_memory_manager::~vulkan_memory_manager()
	{
	}

	void vulkan_memory_manager::allocate_memory(vk::MemoryRequirements memRequirements, vk::MemoryPropertyFlags properties, vulkan_memory &cgbMemory)
	{
		vk::MemoryAllocateInfo allocInfo = {};
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

		if (vulkan_context::instance().device.allocateMemory(&allocInfo, nullptr, &cgbMemory.memory) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		cgbMemory.offset = 0;
	}

	uint32_t vulkan_memory_manager::find_memory_type(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
		vk::PhysicalDeviceMemoryProperties memProperties;
		vulkan_context::instance().physicalDevice.getMemoryProperties(&memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void vulkan_memory_manager::free_memory(vulkan_memory &cgbMemory)
	{
		vkFreeMemory(vulkan_context::instance().device, cgbMemory.memory, nullptr);
	}
}