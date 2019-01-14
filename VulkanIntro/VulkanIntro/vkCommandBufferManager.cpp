#include "vkCommandBufferManager.h"

vkCommandBufferManager::vkCommandBufferManager(vk::CommandPool &commandPool, vk::Queue &transferQueue) : mImageCount(0), mCommandPool(commandPool), mTransferQueue(transferQueue)
{
}

vkCommandBufferManager::vkCommandBufferManager(uint32_t imageCount, vk::CommandPool &commandPool, vk::Queue &transferQueue) : mImageCount(imageCount), mCommandPool(commandPool),
mTransferQueue(transferQueue)
{
	create_command_buffers();
}


vkCommandBufferManager::~vkCommandBufferManager()
{
	vkContext::instance().device.freeCommandBuffers(mCommandPool, static_cast<uint32_t>(mPrimaryCommandBuffers.size()), mPrimaryCommandBuffers.data());
	vkContext::instance().device.freeCommandBuffers(mCommandPool, static_cast<uint32_t>(mSecondaryCommandBuffers.size()), mSecondaryCommandBuffers.data());
}

void vkCommandBufferManager::create_command_buffers() {
	// allocate primary command buffers
	mPrimaryCommandBuffers.resize(mImageCount);

	vk::CommandBufferAllocateInfo allocInfo = {};
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = (uint32_t)mPrimaryCommandBuffers.size();

	if (vkContext::instance().device.allocateCommandBuffers(&allocInfo, mPrimaryCommandBuffers.data()) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	// allocate secondary command buffers
	mSecondaryCommandBuffers.resize(mImageCount);

	allocInfo = {};
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = vk::CommandBufferLevel::eSecondary;
	allocInfo.commandBufferCount = (uint32_t)mSecondaryCommandBuffers.size();

	if (vkContext::instance().device.allocateCommandBuffers(&allocInfo, mSecondaryCommandBuffers.data()) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

vk::CommandBuffer vkCommandBufferManager::get_command_buffer(vk::CommandBufferLevel bufferLevel, vk::CommandBufferBeginInfo &beginInfo) {
	vk::CommandBuffer ret = nullptr;
	if (vk::CommandBufferLevel::ePrimary == bufferLevel) {
		ret = mPrimaryCommandBuffers[vkContext::instance().currentFrame];
	}
	else if (vk::CommandBufferLevel::eSecondary == bufferLevel) {
		ret = mSecondaryCommandBuffers[vkContext::instance().currentFrame];
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

std::vector<vk::CommandBuffer> vkCommandBufferManager::get_recorded_command_buffers(vk::CommandBufferLevel bufferLevel)
{
	std::vector<vk::CommandBuffer> ret;
	if (vk::CommandBufferLevel::ePrimary == bufferLevel) {
		ret.push_back(mPrimaryCommandBuffers[vkContext::instance().currentFrame]);
	}
	else if (vk::CommandBufferLevel::eSecondary == bufferLevel) {
		ret.push_back(mSecondaryCommandBuffers[vkContext::instance().currentFrame]);
	}
	for (vk::CommandBuffer cmdBuf : ret) {
		if (vkEndCommandBuffer(cmdBuf) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
	return ret;
}

vk::CommandBuffer vkCommandBufferManager::begin_single_time_commands() {
	vk::CommandBufferAllocateInfo allocInfo = {};
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer;
	vkContext::instance().device.allocateCommandBuffers(&allocInfo, &commandBuffer);

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	commandBuffer.begin(&beginInfo);

	return commandBuffer;
}

void vkCommandBufferManager::end_single_time_commands(vk::CommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	vk::SubmitInfo submitInfo = {};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	mTransferQueue.submit(1, &submitInfo, nullptr);
	vkQueueWaitIdle(mTransferQueue);

	vkContext::instance().device.freeCommandBuffers(mCommandPool, 1, &commandBuffer);
}