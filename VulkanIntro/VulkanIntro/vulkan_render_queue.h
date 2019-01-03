#pragma once

#include "vkContext.h"

#include <vector>
#include <memory>

#include "vkCommandBufferManager.h"

class vulkan_render_queue
{
public:
	vulkan_render_queue(VkQueue &graphicsQueue, std::shared_ptr<vkCommandBufferManager> drawCommandBufferManager);
	virtual ~vulkan_render_queue();

	void submit(std::vector<VkCommandBuffer> secondaryCommandBuffers, VkFence inFlightFence, std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, VkExtent2D extent);

private:

	VkQueue mGraphicsQueue;
	std::shared_ptr<vkCommandBufferManager> mDrawCommandBufferManager;

	void recordPrimaryCommandBuffer(VkCommandBuffer & commandBuffer, std::vector<VkCommandBuffer> secondaryCommandBuffers, VkExtent2D extent);

};

