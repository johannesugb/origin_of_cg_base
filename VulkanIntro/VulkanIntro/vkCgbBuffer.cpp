#include "vkCgbBuffer.h"

#include <stdexcept>

#include "vkMemoryManager.h"

vkCgbBuffer::vkCgbBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vkCommandBufferManager* commandBufferManager) : 
mCommandBufferManager(commandBufferManager) {
	createBuffer(size, usage, properties, mBuffer, mBufferMemory);
}

vkCgbBuffer::vkCgbBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vkCommandBufferManager* commandBufferManager, void* bufferData) :
	mCommandBufferManager(commandBufferManager) {
	vk::Buffer stagingBuffer;
	vkCgbMemory stagingBufferMemory;
	createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(vkContext::instance().device, stagingBufferMemory.memory, stagingBufferMemory.offset, size, 0, &data);
	memcpy(data, bufferData, (size_t)size);
	vkUnmapMemory(vkContext::instance().device, stagingBufferMemory.memory);

	createBuffer(size, usage, properties, mBuffer, mBufferMemory);

	copy_buffer(stagingBuffer, size);

	vkDestroyBuffer(vkContext::instance().device, stagingBuffer, nullptr);
	vkContext::instance().memoryManager->free_memory(stagingBufferMemory);
}

vkCgbBuffer::~vkCgbBuffer()
{
	vkDestroyBuffer(vkContext::instance().device, mBuffer, nullptr);
	vkContext::instance().memoryManager->free_memory(mBufferMemory);
}

void vkCgbBuffer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vkCgbMemory &cgbMemory) {
	vk::BufferCreateInfo bufferInfo = {};
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	if (vkContext::instance().device.createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create buffer!");
	}

	vk::MemoryRequirements memRequirements;
	vkContext::instance().device.getBufferMemoryRequirements(buffer, &memRequirements);

	vkContext::instance().memoryManager->allocate_memory(memRequirements, properties, cgbMemory);

	vkContext::instance().device.bindBufferMemory(buffer, cgbMemory.memory, cgbMemory.offset);
}

void vkCgbBuffer::copy_buffer(vk::Buffer srcBuffer, vk::DeviceSize size) {
	vk::CommandBuffer commandBuffer = mCommandBufferManager->begin_single_time_commands();

	vk::BufferCopy copyRegion = {};
	copyRegion.size = size;
	commandBuffer.copyBuffer(srcBuffer, mBuffer, 1, &copyRegion);

	mCommandBufferManager->end_single_time_commands(commandBuffer);
}

void vkCgbBuffer::update_buffer(void* bufferData, vk::DeviceSize size)
{
	void* data;
	vkMapMemory(vkContext::instance().device, mBufferMemory.memory, mBufferMemory.offset, size, 0, &data);
	memcpy(data, bufferData, size);
	vkUnmapMemory(vkContext::instance().device, mBufferMemory.memory);
}