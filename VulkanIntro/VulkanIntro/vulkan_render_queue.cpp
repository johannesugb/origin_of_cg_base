#include "vulkan_render_queue.h"

#include <array>

#include "vulkan_framebuffer.h"


vulkan_render_queue::vulkan_render_queue(vk::Queue &graphicsQueue, std::shared_ptr<vkCommandBufferManager> drawCommandBufferManager) : 
	mGraphicsQueue(graphicsQueue), mDrawCommandBufferManager(drawCommandBufferManager)
{
}


vulkan_render_queue::~vulkan_render_queue()
{
}

void vulkan_render_queue::submit(std::vector<vk::CommandBuffer> secondaryCommandBuffers, vk::Fence inFlightFence, std::vector<vk::Semaphore> waitSemaphores, std::vector<vk::Semaphore> signalSemaphores, vk::Extent2D extent)
{
	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	beginInfo.pInheritanceInfo = nullptr; // Optional

	vk::CommandBuffer primCmdBuffer = mDrawCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::ePrimary, beginInfo);
	recordPrimaryCommandBuffer(primCmdBuffer, secondaryCommandBuffers, extent);
	std::vector<vk::CommandBuffer> primaryCommandBuffers = mDrawCommandBufferManager->get_recorded_command_buffers(vk::CommandBufferLevel::ePrimary);

	vk::SubmitInfo submitInfo = {};

	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = waitSemaphores.size();
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = primaryCommandBuffers.size();
	submitInfo.pCommandBuffers = primaryCommandBuffers.data();
	submitInfo.signalSemaphoreCount = signalSemaphores.size();
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	vkContext::instance().device.resetFences(1, &inFlightFence);

	if (mGraphicsQueue.submit(1, &submitInfo, inFlightFence) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void vulkan_render_queue::recordPrimaryCommandBuffer(vk::CommandBuffer &commandBuffer, std::vector<vk::CommandBuffer> secondaryCommandBuffers, vk::Extent2D extent) {
	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = vkContext::instance().vulkanFramebuffer->get_render_pass();
	renderPassInfo.framebuffer = vkContext::instance().vulkanFramebuffer->get_swapchain_framebuffer();

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	std::array<vk::ClearValue, 2> clearValues = {};
	clearValues[0].color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);

	// submit secondary command buffers
	commandBuffer.executeCommands(secondaryCommandBuffers.size(), secondaryCommandBuffers.data());

	commandBuffer.endRenderPass();
}