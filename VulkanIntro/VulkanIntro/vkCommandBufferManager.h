#pragma once
#include "vkContext.h"

#include <vector>
#include <stdexcept>

class vkCommandBufferManager
{
public:
	vkCommandBufferManager(VkCommandPool & commandPool);
	vkCommandBufferManager(uint32_t imageCount, VkCommandPool &commandPool);
	virtual ~vkCommandBufferManager();

	VkCommandBuffer getCommandBuffer(VkCommandBufferLevel bufferLevel, VkCommandBufferBeginInfo &beginInfo);

	std::vector<VkCommandBuffer> getRecordedCommandBuffers(VkCommandBufferLevel bufferLevel);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
private:
	uint32_t _imageCount;

	VkCommandPool _commandPool;
	std::vector<VkCommandBuffer> _secondaryCommandBuffers; // deleted with command pool
	std::vector<VkCommandBuffer> _primaryCommandBuffers; // deleted with command pool

	void createCommandBuffers();
};

