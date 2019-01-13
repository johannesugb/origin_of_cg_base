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
	VkCommandBuffer get_command_buffer(VkCommandBufferLevel bufferLevel, VkCommandBufferBeginInfo &beginInfo);
	std::vector<VkCommandBuffer> get_recorded_command_buffers(VkCommandBufferLevel bufferLevel);

	VkCommandBuffer begin_single_time_commands();
	void end_single_time_commands(VkCommandBuffer commandBuffer);
private:
	uint32_t mImageCount;

	VkCommandPool mCommandPool;
	VkQueue mTransferQueue;

	std::vector<VkCommandBuffer> mSecondaryCommandBuffers; // deleted with command pool
	std::vector<VkCommandBuffer> mPrimaryCommandBuffers; // deleted with command pool

	void create_command_buffers();
};

