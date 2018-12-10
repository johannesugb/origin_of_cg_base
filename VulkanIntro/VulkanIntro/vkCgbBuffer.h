#pragma once
#include "vkContext.h"

#include "vkCgbMemory.h"

#include "vkCommandBufferManager.h"

class vkCgbBuffer
{
public:
	vkCgbBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, vkCommandBufferManager* commandBufferManager);

	// initializes the memory of the buffer with the given data
	// uses a staging buffer to copy the data
	vkCgbBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, vkCommandBufferManager* commandBufferManager, void* data);

	virtual ~vkCgbBuffer();

	VkBuffer getVkBuffer() { return _buffer; }

	void copyBuffer(VkBuffer srcBuffer, VkDeviceSize size);
	void updateBuffer(void* bufferData, VkDeviceSize size);

private:
	VkBuffer _buffer;
	vkCgbMemory _bufferMemory;
	vkCommandBufferManager* _commandBufferManager;

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer & buffer, vkCgbMemory & cgbMemory);
};

