#pragma once
#include "vulkan_context.h"

#include "vulkan_memory.h"

#include "vulkan_command_buffer_manager.h"

namespace cgb {

	class vulkan_buffer
	{
	public:
		// simple standard constructor
		vulkan_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, void* data = nullptr);

		// vulkan native constructor
		// just initializes the internal veriables, does no vulkan stuff
		// use it if you need to make some custom buffers which should be used in the framework
		vulkan_buffer(vk::Buffer buffer, vulkan_memory bufferMemory);


		vulkan_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager = vulkan_context::instance().transferCommandBufferManager);

		// initializes the memory of the buffer with the given data
		// uses a staging buffer to copy the data
		vulkan_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, void* data, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager = vulkan_context::instance().transferCommandBufferManager);

		virtual ~vulkan_buffer();

		vk::Buffer get_vk_buffer() { return mBuffer; }

		// copies the srcBuffer into this buffer with the given size
		void copy_buffer(vk::Buffer srcBuffer, vk::DeviceSize size);

		// updates this buffer with the given buffer data and size
		void update_buffer(void* bufferData, vk::DeviceSize size);

	private:
		vk::Buffer mBuffer;
		vulkan_memory mBufferMemory;
		std::shared_ptr<vulkan_command_buffer_manager> mCommandBufferManager;

		void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer & buffer, vulkan_memory & cgbMemory);
	};

}