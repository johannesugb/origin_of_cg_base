#include "vulkan_command_buffer_manager.h"

namespace cgb {

	vulkan_command_buffer_manager::vulkan_command_buffer_manager(vk::CommandPool &commandPool, vk::Queue &transferQueue) : mImageCount(0), mCommandPool(commandPool), mTransferQueue(transferQueue)
	{
	}

	vulkan_command_buffer_manager::vulkan_command_buffer_manager(uint32_t imageCount, vk::CommandPool &commandPool, vk::Queue &transferQueue) : mImageCount(imageCount), mCommandPool(commandPool),
		mTransferQueue(transferQueue)
	{
		create_command_buffers();
	}


	vulkan_command_buffer_manager::~vulkan_command_buffer_manager()
	{
		for (int i = 0; i < mImageCount; i++) {
			vulkan_context::instance().device.freeCommandBuffers(mCommandPool, static_cast<uint32_t>(mPrimaryCmdBuffers.mFreeCommandBuffers[i].size()), mPrimaryCmdBuffers.mFreeCommandBuffers[i].data());
			vulkan_context::instance().device.freeCommandBuffers(mCommandPool, static_cast<uint32_t>(mPrimaryCmdBuffers.mRecordedCommandBuffers[i].size()), mPrimaryCmdBuffers.mRecordedCommandBuffers[i].data());
			vulkan_context::instance().device.freeCommandBuffers(mCommandPool, static_cast<uint32_t>(mPrimaryCmdBuffers.mSubmittedCommandBuffers[i].size()), mPrimaryCmdBuffers.mSubmittedCommandBuffers[i].data());
			vulkan_context::instance().device.freeCommandBuffers(mCommandPool, static_cast<uint32_t>(mSecondaryCmdBuffers.mFreeCommandBuffers[i].size()), mSecondaryCmdBuffers.mFreeCommandBuffers[i].data());
			vulkan_context::instance().device.freeCommandBuffers(mCommandPool, static_cast<uint32_t>(mSecondaryCmdBuffers.mRecordedCommandBuffers[i].size()), mSecondaryCmdBuffers.mRecordedCommandBuffers[i].data());
			vulkan_context::instance().device.freeCommandBuffers(mCommandPool, static_cast<uint32_t>(mSecondaryCmdBuffers.mSubmittedCommandBuffers[i].size()), mSecondaryCmdBuffers.mSubmittedCommandBuffers[i].data());
		}
	}

	void vulkan_command_buffer_manager::create_command_buffers() {
		mPrimaryCmdBuffers.init_frames(mImageCount);
		mSecondaryCmdBuffers.init_frames(mImageCount);
	}

	vk::CommandBuffer vulkan_command_buffer_manager::get_command_buffer(vk::CommandBufferLevel bufferLevel, vk::CommandBufferBeginInfo &beginInfo) {
		vk::CommandBuffer ret = nullptr;
		if (vk::CommandBufferLevel::ePrimary == bufferLevel) {
			ret = get_or_create_command_buffer(mPrimaryCmdBuffers, bufferLevel, beginInfo);
		}
		else if (vk::CommandBufferLevel::eSecondary == bufferLevel) {
			ret = get_or_create_command_buffer(mSecondaryCmdBuffers, bufferLevel, beginInfo);
		}
		// implictley resets the command buffer before beginning it
		if (ret.begin(&beginInfo) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		if (ret) {
			return ret;
		}
		throw std::runtime_error("No command buffer found for given buffer level!");
	}

	std::vector<vk::CommandBuffer> vulkan_command_buffer_manager::get_recorded_command_buffers(vk::CommandBufferLevel bufferLevel)
	{
		std::vector<vk::CommandBuffer> ret;
		if (vk::CommandBufferLevel::ePrimary == bufferLevel) {
			ret = get_recorded_command_buffers(mPrimaryCmdBuffers);
		}
		else if (vk::CommandBufferLevel::eSecondary == bufferLevel) {
			ret = get_recorded_command_buffers(mSecondaryCmdBuffers);
		}
		for (vk::CommandBuffer cmdBuf : ret) {
			cmdBuf.end();
			//if (vkEndCommandBuffer(cmdBuf) != vk::Result::eSuccess) {
			//	throw std::runtime_error("failed to record command buffer!");
			//}
		}
		return ret;
	}

	void vulkan_command_buffer_manager::reset_command_buffers()
	{
		auto &freePrimCmdBuffer = mPrimaryCmdBuffers.mFreeCommandBuffers[vulkan_context::instance().currentSwapChainIndex];
		auto &finishedPrimCmdBuffer = mPrimaryCmdBuffers.mSubmittedCommandBuffers[vulkan_context::instance().currentSwapChainIndex];
		freePrimCmdBuffer.insert(freePrimCmdBuffer.end(), finishedPrimCmdBuffer.begin(), finishedPrimCmdBuffer.end());
		finishedPrimCmdBuffer.clear();

		auto &freeSecCmdBuffer = mSecondaryCmdBuffers.mFreeCommandBuffers[vulkan_context::instance().currentSwapChainIndex];
		auto &finishedSecCmdBuffer = mSecondaryCmdBuffers.mSubmittedCommandBuffers[vulkan_context::instance().currentSwapChainIndex];
		freeSecCmdBuffer.insert(freeSecCmdBuffer.end(), finishedSecCmdBuffer.begin(), finishedSecCmdBuffer.end());
		finishedSecCmdBuffer.clear();
	}

	vk::CommandBuffer vulkan_command_buffer_manager::begin_single_time_commands() {
		vk::CommandBufferAllocateInfo allocInfo = {};
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = mCommandPool;
		allocInfo.commandBufferCount = 1;

		vk::CommandBuffer commandBuffer;
		vulkan_context::instance().device.allocateCommandBuffers(&allocInfo, &commandBuffer);

		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		commandBuffer.begin(&beginInfo);

		return commandBuffer;
	}

	void vulkan_command_buffer_manager::end_single_time_commands(vk::CommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		vk::SubmitInfo submitInfo = {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		mTransferQueue.submit(1, &submitInfo, nullptr);
		vkQueueWaitIdle(mTransferQueue);

		vulkan_context::instance().device.freeCommandBuffers(mCommandPool, 1, &commandBuffer);
	}

	vk::CommandBuffer vulkan_command_buffer_manager::get_or_create_command_buffer(command_buffer_system &cmdBufferSystem, vk::CommandBufferLevel bufferLevel, vk::CommandBufferBeginInfo &beginInfo)
	{
		auto &freeBuffersForFrame = cmdBufferSystem.mFreeCommandBuffers[vulkan_context::instance().currentSwapChainIndex];
		if (freeBuffersForFrame.empty()) {
			auto newCmdBuffers = std::vector<vk::CommandBuffer>(mImageCount);

			vk::CommandBufferAllocateInfo allocInfo = {};
			allocInfo.commandPool = mCommandPool;
			allocInfo.level = bufferLevel;
			allocInfo.commandBufferCount = (uint32_t)newCmdBuffers.size();

			if (vulkan_context::instance().device.allocateCommandBuffers(&allocInfo, newCmdBuffers.data()) != vk::Result::eSuccess) {
				throw std::runtime_error("failed to allocate command buffers!");
			}
			for (int i = 0; i < mImageCount; i++) {
				cmdBufferSystem.mFreeCommandBuffers[i].push_back(newCmdBuffers[i]);
			}
		}
		auto ret = freeBuffersForFrame.back();
		freeBuffersForFrame.pop_back();
		cmdBufferSystem.mRecordedCommandBuffers[vulkan_context::instance().currentSwapChainIndex].push_back(ret);

		return ret;
	}

	std::vector<vk::CommandBuffer> vulkan_command_buffer_manager::get_recorded_command_buffers(command_buffer_system & cmdBufferSystem)
	{
		auto &submittedBuffersForFrame = cmdBufferSystem.mSubmittedCommandBuffers[vulkan_context::instance().currentSwapChainIndex];
		auto recordedBuffersForFrame = cmdBufferSystem.mRecordedCommandBuffers[vulkan_context::instance().currentSwapChainIndex];
		submittedBuffersForFrame.insert(submittedBuffersForFrame.end(), recordedBuffersForFrame.begin(), recordedBuffersForFrame.end());
		cmdBufferSystem.mRecordedCommandBuffers[vulkan_context::instance().currentSwapChainIndex].clear();

		return recordedBuffersForFrame;
	}
}