#include "vulkan_buffer.h"

#include <stdexcept>

#include "vulkan_memory_manager.h"

namespace cgb {

	vulkan_buffer::vulkan_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, void* data) : 
		vulkan_buffer(size, usage, vk::MemoryPropertyFlagBits::eDeviceLocal, data) {

	}

	vulkan_buffer::vulkan_buffer(vk::Buffer buffer, vulkan_memory bufferMemory) :
	mBuffer(buffer), mBufferMemory(bufferMemory) {
		mCommandBufferManager = vulkan_context::instance().transferCommandBufferManager;
	}

	vulkan_buffer::vulkan_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager) :
		mCommandBufferManager(commandBufferManager) {
		createBuffer(size, usage, properties, mBuffer, mBufferMemory);
	}

	vulkan_buffer::vulkan_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, void* bufferData, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager) :
		mCommandBufferManager(commandBufferManager) {
		vk::Buffer stagingBuffer;
		vulkan_memory stagingBufferMemory;
		createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(vulkan_context::instance().device, stagingBufferMemory.memory, stagingBufferMemory.offset, size, 0, &data);
		memcpy(data, bufferData, (size_t)size);
		vkUnmapMemory(vulkan_context::instance().device, stagingBufferMemory.memory);

		createBuffer(size, usage, properties, mBuffer, mBufferMemory);

		copy_buffer(stagingBuffer, size);

		vkDestroyBuffer(vulkan_context::instance().device, stagingBuffer, nullptr);
		vulkan_context::instance().memoryManager->free_memory(stagingBufferMemory);
	}

	vulkan_buffer::~vulkan_buffer()
	{
		vkDestroyBuffer(vulkan_context::instance().device, mBuffer, nullptr);
		vulkan_context::instance().memoryManager->free_memory(mBufferMemory);
	}

	void vulkan_buffer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vulkan_memory &cgbMemory) {
		vk::BufferCreateInfo bufferInfo = {};
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		if (vulkan_context::instance().device.createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create buffer!");
		}

		vk::MemoryRequirements memRequirements;
		vulkan_context::instance().device.getBufferMemoryRequirements(buffer, &memRequirements);

		vulkan_context::instance().memoryManager->allocate_memory(memRequirements, properties, cgbMemory);

		vulkan_context::instance().device.bindBufferMemory(buffer, cgbMemory.memory, cgbMemory.offset);
	}

	void vulkan_buffer::copy_buffer(vk::Buffer srcBuffer, vk::DeviceSize size) {
		vk::CommandBuffer commandBuffer = mCommandBufferManager->begin_single_time_commands();

		vk::BufferCopy copyRegion = {};
		copyRegion.size = size;
		commandBuffer.copyBuffer(srcBuffer, mBuffer, 1, &copyRegion);

		mCommandBufferManager->end_single_time_commands(commandBuffer);
	}

	void vulkan_buffer::update_buffer(void* bufferData, vk::DeviceSize size)
	{
		void* data;
		vkMapMemory(vulkan_context::instance().device, mBufferMemory.memory, mBufferMemory.offset, size, 0, &data);
		memcpy(data, bufferData, size);
		vkUnmapMemory(vulkan_context::instance().device, mBufferMemory.memory);
	}

}