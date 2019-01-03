#include "vulkan_render_queue.h"

#include <array>


vulkan_render_queue::vulkan_render_queue(VkQueue &graphicsQueue, std::shared_ptr<vkCommandBufferManager> drawCommandBufferManager) : 
	mGraphicsQueue(graphicsQueue), mDrawCommandBufferManager(drawCommandBufferManager)
{
}


vulkan_render_queue::~vulkan_render_queue()
{
}

void vulkan_render_queue::submit(std::vector<VkCommandBuffer> secondaryCommandBuffers, VkFence inFlightFence, std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, VkExtent2D extent)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr; // Optional

	VkCommandBuffer primCmdBuffer = mDrawCommandBufferManager->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, beginInfo);
	recordPrimaryCommandBuffer(primCmdBuffer, secondaryCommandBuffers, extent);
	std::vector<VkCommandBuffer> primaryCommandBuffers = mDrawCommandBufferManager->getRecordedCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = waitSemaphores.size();
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = primaryCommandBuffers.size();
	submitInfo.pCommandBuffers = primaryCommandBuffers.data();
	submitInfo.signalSemaphoreCount = signalSemaphores.size();
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	vkResetFences(vkContext::instance().device, 1, &inFlightFence);

	if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void vulkan_render_queue::recordPrimaryCommandBuffer(VkCommandBuffer &commandBuffer, std::vector<VkCommandBuffer> secondaryCommandBuffers, VkExtent2D extent) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vkContext::instance().renderPass;
	renderPassInfo.framebuffer = vkContext::instance().frameBuffer;

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	// submit secondary command buffers
	vkCmdExecuteCommands(commandBuffer, secondaryCommandBuffers.size(), secondaryCommandBuffers.data());

	vkCmdEndRenderPass(commandBuffer);
}