#pragma once
#include "vkContext.h"

class vkCommandBufferManager
{
public:
	vkCommandBufferManager(VkCommandPool &commandPool);
	virtual ~vkCommandBufferManager();

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
private:

	VkCommandPool _commandPool;
};

