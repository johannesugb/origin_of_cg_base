#pragma once
#include "vkContext.h"

class vkCgbBuffer
{
public:
	vkCgbBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

	// initializes the memory of the buffer with the given data
	// uses a staging buffer to copy the data
	vkCgbBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, void* data);

	virtual ~vkCgbBuffer();

	VkBuffer getVkBuffer() { return _buffer; }

	void copyBuffer(VkBuffer srcBuffer, VkDeviceSize size);
	void updateBuffer(void* bufferData, VkDeviceSize size);

private:
	VkBuffer _buffer;
	VkDeviceMemory _bufferMemory;
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

