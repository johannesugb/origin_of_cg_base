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
		std::shared_ptr<vkCommandBufferManager> drawCommandBufferManager, std::vector<std::shared_ptr<vkRenderer>> predecessors = {}, bool isCompute = false);
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
	std::vector<vk::Semaphore> mImageAvailableSemaphores;
	std::vector<vk::Semaphore> mRenderFinishedSemaphores;
	std::vector<vk::Fence> mInFlightFences;
	static size_t mCurrentFrame; // current frame for synchronization purposes, only used inside this class
	vk::Fence mCurrentInFlightFence;
	std::vector<vk::Semaphore> mCurrentImageAvailableSemaphores;

	bool mSubmitted;
	static vk::CommandBuffer mPrimCmdBuffer;

	void create_sync_objects();
	void recordPrimaryCommandBuffer();

	// no render pass if this is a pure compute renderer (TODO maybe better solution required)
	bool mIsCompute;
};