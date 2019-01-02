#include "vkCommandBufferManager.h"

vkCommandBufferManager::vkCommandBufferManager(VkCommandPool &commandPool, VkQueue &transferQueue) : _imageCount(0), _commandPool(commandPool), mTransferQueue(transferQueue)
{
}

vkCommandBufferManager::vkCommandBufferManager(uint32_t imageCount, VkCommandPool &commandPool, VkQueue &transferQueue) : _imageCount(imageCount), _commandPool(commandPool),
mTransferQueue(transferQueue)
{
	createCommandBuffers();
}


vkCommandBufferManager::~vkCommandBufferManager()
{
	vkFreeCommandBuffers(vkContext::instance().device, _commandPool, static_cast<uint32_t>(_primaryCommandBuffers.size()), _primaryCommandBuffers.data());
	vkFreeCommandBuffers(vkContext::instance().device, _commandPool, static_cast<uint32_t>(_secondaryCommandBuffers.size()), _secondaryCommandBuffers.data());
}

void vkCommandBufferManager::createCommandBuffers() {
	// allocate primary command buffers
	_primaryCommandBuffers.resize(_imageCount);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)_primaryCommandBuffers.size();

	if (vkAllocateCommandBuffers(vkContext::instance().device, &allocInfo, _primaryCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	// allocate secondary command buffers
	_secondaryCommandBuffers.resize(_imageCount);

	allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = (uint32_t)_secondaryCommandBuffers.size();

	if (vkAllocateCommandBuffers(vkContext::instance().device, &allocInfo, _secondaryCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

VkCommandBuffer vkCommandBufferManager::getCommandBuffer(VkCommandBufferLevel bufferLevel, VkCommandBufferBeginInfo &beginInfo) {
	VkCommandBuffer ret = nullptr;
	if (VK_COMMAND_BUFFER_LEVEL_PRIMARY == bufferLevel) {
		ret = _primaryCommandBuffers[vkContext::instance().currentFrame];
	}
	else if (VK_COMMAND_BUFFER_LEVEL_SECONDARY == bufferLevel) {
		ret = _secondaryCommandBuffers[vkContext::instance().currentFrame];
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

std::vector<VkCommandBuffer> vkCommandBufferManager::getRecordedCommandBuffers(VkCommandBufferLevel bufferLevel)
{
	std::vector<VkCommandBuffer> ret;
	if (VK_COMMAND_BUFFER_LEVEL_PRIMARY == bufferLevel) {
		ret.push_back(_primaryCommandBuffers[vkContext::instance().currentFrame]);
	}
	else if (VK_COMMAND_BUFFER_LEVEL_SECONDARY == bufferLevel) {
		ret.push_back(_secondaryCommandBuffers[vkContext::instance().currentFrame]);
	}
	for (VkCommandBuffer cmdBuf : ret) {
		if (vkEndCommandBuffer(cmdBuf) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
	return ret;
}

VkCommandBuffer vkCommandBufferManager::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(vkContext::instance().device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void vkCommandBufferManager::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(mTransferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mTransferQueue);

	vkFreeCommandBuffers(vkContext::instance().device, _commandPool, 1, &commandBuffer);
}