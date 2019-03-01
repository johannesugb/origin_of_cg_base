#pragma once
#include "vulkan_context.h"

#include "vulkan_memory.h"

#include "vulkan_command_buffer_manager.h"

namespace cgb {

	class vulkan_buffer
	{
	public:
		vulkan_buffer() = default;
		vulkan_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vulkan_command_buffer_manager* commandBufferManager);

		// initializes the memory of the buffer with the given data
		// uses a staging buffer to copy the data
		vulkan_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vulkan_command_buffer_manager* commandBufferManager, void* data);

		virtual ~vulkan_buffer();

		vk::Buffer get_vkk_buffer() { return mBuffer; }

		void copy_buffer(vk::Buffer srcBuffer, vk::DeviceSize size);
		void update_buffer(void* bufferData, vk::DeviceSize size);

	private:
		vk::Buffer mBuffer;
		vulkan_memory mBufferMemory;
		vulkan_command_buffer_manager* mCommandBufferManager;

		void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer & buffer, vulkan_memory & cgbMemory);
	};

}