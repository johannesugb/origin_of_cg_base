#pragma once
#include "vulkan_context.h"

#include "vulkan_memory.h"

#include <unordered_map>

namespace cgb {
	class vulkan_memory_manager
	{
	public:
		vulkan_memory_manager();
		virtual ~vulkan_memory_manager();

		void allocate_memory(vk::MemoryRequirements memRequirements, vk::MemoryPropertyFlags properties, vulkan_memory &cgbMemory);
		void free_memory(vulkan_memory &cgbMemory);

		// cleans up all allocated memory !! ATTENTION !! if the memory is still bound or used afterwards this might result in undefined behavior
		void cleanup();
	private:
		uint32_t find_memory_type(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

		std::unordered_map<vk::DeviceMemory*, vulkan_memory> allocatedMemory;
	};

}