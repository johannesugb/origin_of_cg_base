#pragma once
#include "vkContext.h"

#include <vector>
#include <stdexcept>

class vkCommandBufferManager
{
public:
	vkCommandBufferManager(vk::CommandPool & commandPool, vk::Queue &transferQueue);
	vkCommandBufferManager(uint32_t imageCount, vk::CommandPool &commandPool, vk::Queue &transferQueue);
	virtual ~vkCommandBufferManager();

	// TODO better command buffer management, do not always begin command buffer, only if it has not begun yet, etc.
	vk::CommandBuffer get_command_buffer(vk::CommandBufferLevel bufferLevel, vk::CommandBufferBeginInfo &beginInfo);
	std::vector<vk::CommandBuffer> get_recorded_command_buffers(vk::CommandBufferLevel bufferLevel);

	vk::CommandBuffer begin_single_time_commands();
	void end_single_time_commands(vk::CommandBuffer commandBuffer);
private:
	uint32_t mImageCount;

	vk::CommandPool mCommandPool;
	vk::Queue mTransferQueue;

	std::vector<vk::CommandBuffer> mSecondaryCommandBuffers; // deleted with command pool
	std::vector<vk::CommandBuffer> mPrimaryCommandBuffers; // deleted with command pool

	void create_command_buffers();
};

