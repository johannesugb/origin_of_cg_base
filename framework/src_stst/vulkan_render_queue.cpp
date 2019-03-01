#include "vulkan_render_queue.h"

#include <array>

#include "vulkan_framebuffer.h"

namespace cgb {

	vulkan_render_queue::vulkan_render_queue(vk::Queue &graphicsQueue) :
		mGraphicsQueue(graphicsQueue)
	{
	}


	vulkan_render_queue::~vulkan_render_queue()
	{
	}

	void vulkan_render_queue::submit(std::vector<vk::CommandBuffer> commandBuffers, vk::Fence inFlightFence, std::vector<vk::Semaphore> waitSemaphores, std::vector<vk::Semaphore> signalSemaphores)
	{
		vk::SubmitInfo submitInfo = {};

		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = commandBuffers.size();
		submitInfo.pCommandBuffers = commandBuffers.data();
		submitInfo.signalSemaphoreCount = signalSemaphores.size();
		submitInfo.pSignalSemaphores = signalSemaphores.data();

		vulkan_context::instance().device.resetFences(1, &inFlightFence);

		auto result = mGraphicsQueue.submit(1, &submitInfo, inFlightFence);
		if (result != vk::Result::eSuccess) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	}
}