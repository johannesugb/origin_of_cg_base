#include "vkRenderer.h"


size_t vkRenderer::mCurrentFrame = 0;

// TODO Renderer Predecessors and their signal semaphores
vkRenderer::vkRenderer(std::shared_ptr<vkImagePresenter> imagePresenter, std::shared_ptr<vulkan_render_queue> vulkanRenderQueue,
	std::shared_ptr<vkCommandBufferManager> drawCommandBufferManager, std::vector<std::shared_ptr<vkRenderer>> predecessors) :
	mImagePresenter(imagePresenter), mVulkanRenderQueue(vulkanRenderQueue), mDrawCommandBufferManager(drawCommandBufferManager),
	mPredecessors(predecessors)
{
	mCurrentInFlightFence = nullptr;
	mSubmitted = false;
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
	// first submit all predeccessors
	for (std::shared_ptr<vkRenderer> renderer : mPredecessors) {
		renderer->submit_render();
		mCurrentImageAvailableSemaphores.push_back(renderer->mRenderFinishedSemaphores[mCurrentFrame]);
	}
	drawer->draw(renderObjects);
	mSubmitted = false;
}

void vkRenderer::end_frame()
{
	submit_render();
	mImagePresenter->present_image({ mRenderFinishedSemaphores[mCurrentFrame] });
	mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void vkRenderer::submit_render()
{
	if (!mSubmitted) {
		std::vector<vk::CommandBuffer> secondaryCommandBuffers = mDrawCommandBufferManager->get_recorded_command_buffers(vk::CommandBufferLevel::eSecondary);
		mVulkanRenderQueue->submit(secondaryCommandBuffers, mCurrentInFlightFence, { mCurrentImageAvailableSemaphores }, { mRenderFinishedSemaphores[mCurrentFrame] }, mImagePresenter->get_swap_chain_extent());
		// reset fence to null handle, if this an intermediate renderer it will not have started a frame and therefore the fence wil stay null -> 
		// no unnecessary fence is submitted to the queue
		mCurrentInFlightFence = nullptr;
		// reset waiting semaphores, because we get new ones for the next frame
		mCurrentImageAvailableSemaphores.clear();
		
		mSubmitted = true;
		// maybe trim command pool each minute or so
		// vkTrimCommandPool
	}
}

void vkRenderer::create_sync_objects() {
	mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	vk::SemaphoreCreateInfo semaphoreInfo = {};

	vk::FenceCreateInfo fenceInfo = {};
	fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkContext::instance().device.createSemaphore(&semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != vk::Result::eSuccess ||
			vkContext::instance().device.createSemaphore(&semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != vk::Result::eSuccess ||
			vkContext::instance().device.createFence(&fenceInfo, nullptr, &mInFlightFences[i]) != vk::Result::eSuccess) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}