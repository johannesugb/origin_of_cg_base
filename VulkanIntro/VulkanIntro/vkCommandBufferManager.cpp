#include "vkCommandBufferManager.h"

vkCommandBufferManager::vkCommandBufferManager(VkCommandPool &commandPool, VkQueue &transferQueue) : mImageCount(0), mCommandPool(commandPool), mTransferQueue(transferQueue)
{
}

vkCommandBufferManager::vkCommandBufferManager(uint32_t imageCount, VkCommandPool &commandPool, VkQueue &transferQueue) : mImageCount(imageCount), mCommandPool(commandPool),
mTransferQueue(transferQueue)
{
	create_command_buffers();
}


vkCommandBufferManager::~vkCommandBufferManager()
{
	vkFreeCommandBuffers(vkContext::instance().device, mCommandPool, static_cast<uint32_t>(mPrimaryCommandBuffers.size()), mPrimaryCommandBuffers.data());
	vkFreeCommandBuffers(vkContext::instance().device, mCommandPool, static_cast<uint32_t>(mSecondaryCommandBuffers.size()), mSecondaryCommandBuffers.data());
}

void vkCommandBufferManager::create_command_buffers() {
	// allocate primary command buffers
	mPrimaryCommandBuffers.resize(mImageCount);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)mPrimaryCommandBuffers.size();

	if (vkAllocateCommandBuffers(vkContext::instance().device, &allocInfo, mPrimaryCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	// allocate secondary command buffers
	mSecondaryCommandBuffers.resize(mImageCount);

	allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = (uint32_t)mSecondaryCommandBuffers.size();

	if (vkAllocateCommandBuffers(vkContext::instance().device, &allocInfo, mSecondaryCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

VkCommandBuffer vkCommandBufferManager::get_command_buffer(VkCommandBufferLevel bufferLevel, VkCommandBufferBeginInfo &beginInfo) {
	VkCommandBuffer ret = nullptr;
	if (VK_COMMAND_BUFFER_LEVEL_PRIMARY == bufferLevel) {
		ret = mPrimaryCommandBuffers[vkContext::instance().currentFrame];
	}
	else if (VK_COMMAND_BUFFER_LEVEL_SECONDARY == bufferLevel) {
		ret = mSecondaryCommandBuffers[vkContext::instance().currentFrame];
	}
	// implictley resets the command buffer before beginning it
	if (vkBeginCommandBuffer(ret, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}
	if (ret) {
		return ret;
	}
	throw std::runtime_error("No command buffer found for given buffer level!");
}

std::vector<VkCommandBuffer> vkCommandBufferManager::get_recorded_command_buffers(VkCommandBufferLevel bufferLevel)
{
	std::vector<VkCommandBuffer> ret;
	if (VK_COMMAND_BUFFER_LEVEL_PRIMARY == bufferLevel) {
		ret.push_back(mPrimaryCommandBuffers[vkContext::instance().currentFrame]);
	}
	else if (VK_COMMAND_BUFFER_LEVEL_SECONDARY == bufferLevel) {
		ret.push_back(mSecondaryCommandBuffers[vkContext::instance().currentFrame]);
	}
	for (VkCommandBuffer cmdBuf : ret) {
		if (vkEndCommandBuffer(cmdBuf) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
	return ret;
}

VkCommandBuffer vkCommandBufferManager::begin_single_time_commands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(vkContext::instance().device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void vkCommandBufferManager::end_single_time_commands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(mTransferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mTransferQueue);

	vkFreeCommandBuffers(vkContext::instance().device, mCommandPool, 1, &commandBuffer);
}