#pragma once
#include "vkContext.h"

#include "vkCgbMemory.h"

#include "vkCommandBufferManager.h"

class vkCgbBuffer
{
public:
	vkCgbBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vkCommandBufferManager* commandBufferManager);

	// initializes the memory of the buffer with the given data
	// uses a staging buffer to copy the data
	vkCgbBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vkCommandBufferManager* commandBufferManager, void* data);

	virtual ~vkCgbBuffer();

	vk::Buffer get_vkk_buffer() { return mBuffer; }

	void copy_buffer(vk::Buffer srcBuffer, vk::DeviceSize size);
	void update_buffer(void* bufferData, vk::DeviceSize size);

private:
	vk::Buffer mBuffer;
	vkCgbMemory mBufferMemory;
	vkCommandBufferManager* mCommandBufferManager;

	void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer & buffer, vkCgbMemory & cgbMemory);
};

