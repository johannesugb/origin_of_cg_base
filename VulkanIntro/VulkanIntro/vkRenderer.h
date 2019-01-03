#pragma once

#include "vkContext.h"

#include <vector>

#include "vkRenderObject.h"
#include "vkDrawer.h"
#include "vkImagePresenter.h"
#include "vulkan_render_queue.h"

class vkRenderer
{
public:
	vkRenderer(std::shared_ptr<vkImagePresenter> imagePresenter, std::shared_ptr<vulkan_render_queue> vulkanRenderQueue,
		std::shared_ptr<vkCommandBufferManager> drawCommandBufferManager);
	virtual ~vkRenderer();

	void start_frame();
	void render(std::vector<vkRenderObject*> renderObjects, vkDrawer* drawer);
	void end_frame();

private:
	std::shared_ptr<vkImagePresenter> mImagePresenter;
	std::shared_ptr<vulkan_render_queue> mVulkanRenderQueue;
	std::shared_ptr<vkCommandBufferManager> mDrawCommandBufferManager;

	// synchronization
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mInFlightFences;
	size_t mCurrentFrame; // current frame for synchronization purposes, only used inside this class
	VkFence mCurrentInFlightFence;
	std::vector<VkSemaphore> mCurrentImageAvailableSemaphores;

	void submit_render();

	void create_sync_objects();
};

