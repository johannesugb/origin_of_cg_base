#pragma once

#include "vkContext.h"

#include <vector>
#include <memory>

#include "vkCommandBufferManager.h"

class vulkan_render_queue
{
public:
	vulkan_render_queue(vk::Queue &graphicsQueue, std::shared_ptr<vkCommandBufferManager> drawCommandBufferManager);
	virtual ~vulkan_render_queue();

	void submit(std::vector<vk::CommandBuffer> secondaryCommandBuffers, vk::Fence inFlightFence, std::vector<vk::Semaphore> waitSemaphores, std::vector<vk::Semaphore> signalSemaphores, vk::Extent2D extent);

private:

	vk::Queue mGraphicsQueue;
	std::shared_ptr<vkCommandBufferManager> mDrawCommandBufferManager;

	void recordPrimaryCommandBuffer(vk::CommandBuffer & commandBuffer, std::vector<vk::CommandBuffer> secondaryCommandBuffers, vk::Extent2D extent);

};

