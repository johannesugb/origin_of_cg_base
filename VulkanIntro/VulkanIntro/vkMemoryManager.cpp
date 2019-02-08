#include "vkMemoryManager.h"

#include <stdexcept>


vkMemoryManager::vkMemoryManager()
{
}


vkMemoryManager::~vkMemoryManager()
{
}

void vkMemoryManager::allocate_memory(vk::MemoryRequirements memRequirements, vk::MemoryPropertyFlags properties, vkCgbMemory &cgbMemory)
{
	vk::MemoryAllocateInfo allocInfo = {};
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

	if (vkContext::instance().device.allocateMemory(&allocInfo, nullptr, &cgbMemory.memory) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	cgbMemory.offset = 0;
}

uint32_t vkMemoryManager::find_memory_type(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
	vk::PhysicalDeviceMemoryProperties memProperties;
	vkContext::instance().physicalDevice.getMemoryProperties(&memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void vkMemoryManager::free_memory(vkCgbMemory &cgbMemory)
{
	vkFreeMemory(vkContext::instance().device, cgbMemory.memory, nullptr);
}