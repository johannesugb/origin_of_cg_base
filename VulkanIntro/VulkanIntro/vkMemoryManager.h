#pragma once
#include "vkContext.h"

#include "vkCgbMemory.h"

class vkMemoryManager
{
public:
	vkMemoryManager();
	virtual ~vkMemoryManager();

	void allocate_memory(vk::MemoryRequirements memRequirements, vk::MemoryPropertyFlags properties, vkCgbMemory &cgbMemory);
	void free_memory(vkCgbMemory &cgbMemory);
private:
	uint32_t find_memory_type(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
};

