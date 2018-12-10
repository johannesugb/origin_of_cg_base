#pragma once
#include "vkContext.h"

#include "vkCgbMemory.h"

class vkMemoryManager
{
public:
	vkMemoryManager();
	virtual ~vkMemoryManager();

	void allocateMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties, vkCgbMemory &cgbMemory);
	void freeMemory(vkCgbMemory &cgbMemory);
private:
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

