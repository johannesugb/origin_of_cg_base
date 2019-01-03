#include "vkRenderer.h"


// TODO Renderer Predecessors and their signal semaphores
vkRenderer::vkRenderer(std::shared_ptr<vkImagePresenter> imagePresenter, std::shared_ptr<vulkan_render_queue> vulkanRenderQueue,
	std::shared_ptr<vkCommandBufferManager> drawCommandBufferManager) : 
	mImagePresenter(imagePresenter), mVulkanRenderQueue(vulkanRenderQueue), mDrawCommandBufferManager(drawCommandBufferManager)
{
	mCurrentFrame = 0; 
	mCurrentInFlightFence = VK_NULL_HANDLE;
	create_sync_objects();
}


vkRenderer::~vkRenderer()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(vkContext::instance().device, mRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(vkContext::instance().device, mImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(vkContext::instance().device, mInFlightFences[i], nullptr);
	}
}

void vkRenderer::start_frame()
{
	mCurrentInFlightFence = mInFlightFences[mCurrentFrame];
	mCurrentImageAvailableSemaphores.push_back(mImageAvailableSemaphores[mCurrentFrame]);
	mImagePresenter->fetch_next_swapchain_image(mCurrentInFlightFence, mImageAvailableSemaphores[mCurrentFrame]);
}

void vkRenderer::render(std::vector<vkRenderObject*> renderObjects, vkDrawer * drawer)
{
	drawer->draw(renderObjects);
}

void vkRenderer::end_frame()
{
	submit_render();
	mImagePresenter->present_image({ mRenderFinishedSemaphores[mCurrentFrame] });
	mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void vkRenderer::submit_render()
{
	std::vector<VkCommandBuffer> secondaryCommandBuffers = mDrawCommandBufferManager->getRecordedCommandBuffers(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	mVulkanRenderQueue->submit(secondaryCommandBuffers, mCurrentInFlightFence, { mCurrentImageAvailableSemaphores }, { mRenderFinishedSemaphores[mCurrentFrame] }, mImagePresenter->get_swap_chain_extent());
	// reset fence to null handle, if this an intermediate renderer it will not have started a frame and therefore the fence wil stay null -> 
	// no unnecessary fence is submitted to the queue
	mCurrentInFlightFence = VK_NULL_HANDLE; 
	// reset waiting semaphores, because we get new ones for the next frame
	mCurrentImageAvailableSemaphores.clear();
	// maybe trim command pool each minute or so
	// vkTrimCommandPool
}

void vkRenderer::create_sync_objects() {
	mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(vkContext::instance().device, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(vkContext::instance().device, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(vkContext::instance().device, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}