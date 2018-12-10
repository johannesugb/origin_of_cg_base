#pragma once
#include "vkContext.h"

class vkCgbMemory
{
public:
	vkCgbMemory();
	~vkCgbMemory();

	VkDeviceMemory memory;
	VkDeviceSize offset;
};

