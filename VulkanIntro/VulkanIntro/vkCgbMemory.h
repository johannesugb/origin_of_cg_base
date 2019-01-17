#pragma once
#include "vkContext.h"

class vkCgbMemory
{
public:
	vkCgbMemory();
	~vkCgbMemory();

	vk::DeviceMemory memory;
	vk::DeviceSize offset;
};

