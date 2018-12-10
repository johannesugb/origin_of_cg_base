#include "vkCgbBuffer.h"

#include <stdexcept>

#include "vkMemoryManager.h"

vkCgbBuffer::vkCgbBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, vkCommandBufferManager* commandBufferManager) : 
_commandBufferManager(commandBufferManager) {
	createBuffer(size, usage, properties, _buffer, _bufferMemory);
}

vkCgbBuffer::vkCgbBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, vkCommandBufferManager* commandBufferManager, void* bufferData) :
	_commandBufferManager(commandBufferManager) {
	VkBuffer stagingBuffer;
	vkCgbMemory stagingBufferMemory;
	createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(vkContext::instance().device, stagingBufferMemory.memory, stagingBufferMemory.offset, size, 0, &data);
	memcpy(data, bufferData, (size_t)size);
	vkUnmapMemory(vkContext::instance().device, stagingBufferMemory.memory);

	createBuffer(size, usage, properties, _buffer, _bufferMemory);

	copyBuffer(stagingBuffer, size);

	vkDestroyBuffer(vkContext::instance().device, stagingBuffer, nullptr);
	vkContext::instance().memoryManager->freeMemory(stagingBufferMemory);
}

vkCgbBuffer::~vkCgbBuffer()
{
	vkDestroyBuffer(vkContext::instance().device, _buffer, nullptr);
	vkContext::instance().memoryManager->freeMemory(_bufferMemory);
}

void vkCgbBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, vkCgbMemory &cgbMemory) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vkContext::instance().device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(vkContext::instance().device, buffer, &memRequirements);

	vkContext::instance().memoryManager->allocateMemory(memRequirements, properties, cgbMemory);

	vkBindBufferMemory(vkContext::instance().device, buffer, cgbMemory.memory, cgbMemory.offset);
}

void vkCgbBuffer::copyBuffer(VkBuffer srcBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = _commandBufferManager->beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, _buffer, 1, &copyRegion);

	_commandBufferManager->endSingleTimeCommands(commandBuffer);
}

void vkCgbBuffer::updateBuffer(void* bufferData, VkDeviceSize size)
{
	void* data;
	vkMapMemory(vkContext::instance().device, _bufferMemory.memory, _bufferMemory.offset, size, 0, &data);
	memcpy(data, bufferData, size);
	vkUnmapMemory(vkContext::instance().device, _bufferMemory.memory);
}