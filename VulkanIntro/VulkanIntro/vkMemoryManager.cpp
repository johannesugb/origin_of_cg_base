#include "vkMemoryManager.h"

#include <stdexcept>


vkMemoryManager::vkMemoryManager()
{
}


vkMemoryManager::~vkMemoryManager()
{
}

void vkMemoryManager::allocateMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties, vkCgbMemory &cgbMemory)
{
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(vkContext::instance().device, &allocInfo, nullptr, &cgbMemory.memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	cgbMemory.offset = 0;
}

uint32_t vkMemoryManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(vkContext::instance().physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;

		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void vkMemoryManager::freeMemory(vkCgbMemory &cgbMemory)
{
	vkFreeMemory(vkContext::instance().device, cgbMemory.memory, nullptr);
}