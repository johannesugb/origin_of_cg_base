#include "vkCgbBuffer.h"

#include <stdexcept>


vkCgbBuffer::vkCgbBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
	createBuffer(size, usage, properties, _buffer, _bufferMemory);
}

vkCgbBuffer::vkCgbBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, void* bufferData) {
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(vkContext::instance().device, stagingBufferMemory, 0, size, 0, &data);
	memcpy(data, bufferData, (size_t)size);
	vkUnmapMemory(vkContext::instance().device, stagingBufferMemory);

	createBuffer(size, usage, properties, _buffer, _bufferMemory);

	copyBuffer(stagingBuffer, size);

	vkDestroyBuffer(vkContext::instance().device, stagingBuffer, nullptr);
	vkFreeMemory(vkContext::instance().device, stagingBufferMemory, nullptr);
}

vkCgbBuffer::~vkCgbBuffer()
{
	vkDestroyBuffer(vkContext::instance().device, _buffer, nullptr);
	vkFreeMemory(vkContext::instance().device, _bufferMemory, nullptr);
}

void vkCgbBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(vkContext::instance().device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(vkContext::instance().device, buffer, bufferMemory, 0);
}

uint32_t vkCgbBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(vkContext::instance().physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;

		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void vkCgbBuffer::copyBuffer(VkBuffer srcBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, _buffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}

void vkCgbBuffer::updateBuffer(void* bufferData, VkDeviceSize size)
{
	void* data;
	vkMapMemory(vkContext::instance().device, _bufferMemory, 0, size, 0, &data);
	memcpy(data, bufferData, size);
	vkUnmapMemory(vkContext::instance().device, _bufferMemory);
}

VkCommandBuffer vkCgbBuffer::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vkContext::instance().transferCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(vkContext::instance().device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void vkCgbBuffer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(vkContext::instance().graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vkContext::instance().graphicsQueue);

	vkFreeCommandBuffers(vkContext::instance().device, vkContext::instance().transferCommandPool, 1, &commandBuffer);
}