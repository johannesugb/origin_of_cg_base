#pragma once

#include "vkContext.h"

#include <vector>
#include <memory>

#include "vkRenderObject.h"
#include "vkDrawer.h"
#include "vkImagePresenter.h"
#include "vulkan_render_queue.h"

class vkRenderer
{
public:
	vkRenderer(std::shared_ptr<vkImagePresenter> imagePresenter, std::shared_ptr<vulkan_render_queue> vulkanRenderQueue,
		std::shared_ptr<vkCommandBufferManager> drawCommandBufferManager, std::vector<std::shared_ptr<vkRenderer>> predecessors = {});
	virtual ~vkRenderer();

	void start_frame();
	void render(std::vector<vkRenderObject*> renderObjects, vkDrawer* drawer);
	void end_frame();

	// manually submit renderer, not needed for OpenGl, done automatically on end_frame or if rendering a successor
	void submit_render();

private:
	std::shared_ptr<vkImagePresenter> mImagePresenter;
	std::shared_ptr<vulkan_render_queue> mVulkanRenderQueue;
	std::shared_ptr<vkCommandBufferManager> mDrawCommandBufferManager;
	std::vector<std::shared_ptr<vkRenderer>> mPredecessors;

	// synchronization
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mInFlightFences;
	static size_t mCurrentFrame; // current frame for synchronization purposes, only used inside this class
	VkFence mCurrentInFlightFence;
	std::vector<VkSemaphore> mCurrentImageAvailableSemaphores;

	bool mSubmitted;

	void create_sync_objects();
};