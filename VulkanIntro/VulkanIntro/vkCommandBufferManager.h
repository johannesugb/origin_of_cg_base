#pragma once
#include "vkContext.h"

#include <vector>
#include <stdexcept>

class vkCommandBufferManager
{
public:
	vkCommandBufferManager(VkCommandPool & commandPool, VkQueue &transferQueue);
	vkCommandBufferManager(uint32_t imageCount, VkCommandPool &commandPool, VkQueue &transferQueue);
	virtual ~vkCommandBufferManager();

	// TODO better command buffer management, do not always begin command buffer, only if it has not begun yet, etc.
	VkCommandBuffer getCommandBuffer(VkCommandBufferLevel bufferLevel, VkCommandBufferBeginInfo &beginInfo);
	std::vector<VkCommandBuffer> getRecordedCommandBuffers(VkCommandBufferLevel bufferLevel);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
private:
	uint32_t _imageCount;

	VkCommandPool _commandPool;
	VkQueue mTransferQueue;

	std::vector<VkCommandBuffer> _secondaryCommandBuffers; // deleted with command pool
	std::vector<VkCommandBuffer> _primaryCommandBuffers; // deleted with command pool

	void createCommandBuffers();
};

