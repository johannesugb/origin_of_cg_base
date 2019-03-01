#pragma once

#include "vulkan_context.h"

#include <vector>
#include <memory>

#include "vulkan_command_buffer_manager.h"

namespace cgb {

	class vulkan_render_queue
	{
	public:
		vulkan_render_queue(vk::Queue &graphicsQueue);
		virtual ~vulkan_render_queue();

		void submit(std::vector<vk::CommandBuffer> commandBuffers, vk::Fence inFlightFence, std::vector<vk::Semaphore> waitSemaphores, std::vector<vk::Semaphore> signalSemaphores);

	private:

		vk::Queue mGraphicsQueue;
	};

}