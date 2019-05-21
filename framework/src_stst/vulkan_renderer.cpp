#include "vulkan_renderer.h"

#include "vulkan_framebuffer.h"

namespace cgb {

	size_t vulkan_renderer::mCurrentFrame = 0;
	vk::CommandBuffer vulkan_renderer::mPrimCmdBuffer = nullptr;

	// TODO Renderer Predecessors and their signal semaphores
	vulkan_renderer::vulkan_renderer(std::shared_ptr<vulkan_image_presenter> imagePresenter, std::shared_ptr<vulkan_render_queue> vulkanRenderQueue,
		std::shared_ptr<vulkan_command_buffer_manager> drawCommandBufferManager, std::vector<std::shared_ptr<vulkan_renderer>> predecessors, bool isCompute) :
		mImagePresenter(imagePresenter), mVulkanRenderQueue(vulkanRenderQueue), mDrawCommandBufferManager(drawCommandBufferManager),
		mPredecessors(predecessors), mIsCompute(isCompute)
	{
		mCurrentInFlightFence = nullptr;
		mSubmitted = false;
	}


	vulkan_renderer::~vulkan_renderer()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(vulkan_context::instance().device, mRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(vulkan_context::instance().device, mImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(vulkan_context::instance().device, mInFlightFences[i], nullptr);
		}
	}

	void vulkan_renderer::start_frame()
	{
		mCurrentInFlightFence = mInFlightFences[mCurrentFrame];
		mCurrentImageAvailableSemaphores.push_back(mImageAvailableSemaphores[mCurrentFrame]);
		mImagePresenter->fetch_next_swapchain_image(mCurrentInFlightFence, mImageAvailableSemaphores[mCurrentFrame]);

		// reset all command buffers for this frame
		mDrawCommandBufferManager->reset_command_buffers();

		// begin primary CommandBuffer for this frame
		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
		beginInfo.pInheritanceInfo = nullptr; // Optional

		mPrimCmdBuffer = mDrawCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::ePrimary, beginInfo);
	}

	void vulkan_renderer::render(std::vector<vulkan_render_object*> renderObjects, vulkan_drawer * drawer)
	{
		// first submit all predeccessors
		for (std::shared_ptr<vulkan_renderer> renderer : mPredecessors) {
			// TODO FUTURE, check if previous renderer uses same submission queue
			// then just record primary command buffer
			// otherwise submit and use semaphores!
			renderer->recordPrimaryCommandBuffer();
			//mCurrentImageAvailableSemaphores.push_back(renderer->mRenderFinishedSemaphores[mCurrentFrame]);
		}
		drawer->draw(renderObjects);
		mSubmitted = false;
	}

	void vulkan_renderer::end_frame()
	{
		submit_render();
		mImagePresenter->present_image({ mRenderFinishedSemaphores[mCurrentFrame] });
		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void vulkan_renderer::submit_render()
	{
		recordPrimaryCommandBuffer();
		std::vector<vk::CommandBuffer> primaryCommandBuffers = mDrawCommandBufferManager->get_recorded_command_buffers(vk::CommandBufferLevel::ePrimary);
		mVulkanRenderQueue->submit(primaryCommandBuffers, mCurrentInFlightFence, { mCurrentImageAvailableSemaphores }, { mRenderFinishedSemaphores[mCurrentFrame] });
		// reset fence to null handle, if this an intermediate renderer it will not have started a frame and therefore the fence wil stay null -> 
		// no unnecessary fence is submitted to the queue
		mCurrentInFlightFence = nullptr;
		// reset waiting semaphores, because we get new ones for the next frame
		mCurrentImageAvailableSemaphores.clear();

		// maybe trim command pool each minute or so
		// vkTrimCommandPool
	}

	void vulkan_renderer::recordPrimaryCommandBuffer() {
		if (!mSubmitted) {
			std::vector<vk::CommandBuffer> secondaryCommandBuffers = mDrawCommandBufferManager->get_recorded_command_buffers(vk::CommandBufferLevel::eSecondary);

			// no render pass if this is a pure compute renderer (maybe better solution required) 
			if (!mIsCompute) {
				vk::RenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.renderPass = vulkan_context::instance().vulkanFramebuffer->get_render_pass();
				renderPassInfo.framebuffer = vulkan_context::instance().vulkanFramebuffer->get_swapchain_framebuffer();

				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = vulkan_context::instance().vulkanFramebuffer->get_framebuffer_extent();

				std::array<vk::ClearValue, 2> clearValues = {};
				clearValues[0].color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
				clearValues[1].depthStencil = { 1.0f, 0 };
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();

				mPrimCmdBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
			}

			// submit secondary command buffers
			mPrimCmdBuffer.executeCommands(secondaryCommandBuffers.size(), secondaryCommandBuffers.data());

			if (!mIsCompute) {
				mPrimCmdBuffer.endRenderPass();
			}

			mSubmitted = true;
		}
	}
}