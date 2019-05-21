#pragma once

#include "vulkan_context.h"

#include <vector>
#include <memory>

#include "vulkan_render_object.h"
#include "vulkan_drawer.h"
#include "vulkan_image_presenter.h"
#include "vulkan_render_queue.h"

namespace cgb {

	class vulkan_renderer
	{
	public:
		vulkan_renderer(std::shared_ptr<vulkan_image_presenter> imagePresenter, std::shared_ptr<vulkan_render_queue> vulkanRenderQueue,
			std::shared_ptr<vulkan_command_buffer_manager> drawCommandBufferManager, std::vector<std::shared_ptr<vulkan_renderer>> predecessors = {}, bool isCompute = false);
		virtual ~vulkan_renderer();

		void start_frame();
		void render(std::vector<vulkan_render_object*> renderObjects, vulkan_drawer* drawer);
		void end_frame();

		// manually submit renderer, not needed for OpenGl, done automatically on end_frame or if rendering a successor
		void submit_render();

	private:
		std::shared_ptr<vulkan_image_presenter> mImagePresenter;
		std::shared_ptr<vulkan_render_queue> mVulkanRenderQueue;
		std::shared_ptr<vulkan_command_buffer_manager> mDrawCommandBufferManager;
		std::vector<std::shared_ptr<vulkan_renderer>> mPredecessors;

		// synchronization
		std::vector<vk::Semaphore> mImageAvailableSemaphores;
		std::vector<vk::Semaphore> mRenderFinishedSemaphores;
		std::vector<vk::Fence> mInFlightFences;
		static size_t mCurrentFrame; // current frame for synchronization purposes, only used inside this class
		vk::Fence mCurrentInFlightFence;
		std::vector<vk::Semaphore> mCurrentImageAvailableSemaphores;

		bool mSubmitted;
		static vk::CommandBuffer mPrimCmdBuffer;

		void recordPrimaryCommandBuffer();

		// no render pass if this is a pure compute renderer (TODO maybe better solution required)
		bool mIsCompute;
	};
}