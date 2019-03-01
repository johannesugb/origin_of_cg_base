#pragma once
#include "vulkan_context.h"

#include <vector>
#include <stdexcept>
#include <unordered_map>

namespace std {
	template<typename T> struct hash<vk::Flags<T, VkFlags>> { //CommandBufferUsageFlagBits
		size_t operator()(vk::Flags<T, VkFlags> const& flags) const {
			VkFlags flag = static_cast<VkFlags>(flags);
			return hash<VkFlags>()(flag);
		}
	};

	template<> struct hash<vk::CommandBufferBeginInfo> {
		size_t operator()(vk::CommandBufferBeginInfo const& beginInfo) const {
			return ((hash<vk::Flags<vk::CommandBufferUsageFlagBits, VkFlags>>()(beginInfo.flags)));
		}
	};
}

namespace cgb {
	// a vector for the frames, a map for the BeginInfos and a vector for multiple commandbuffers per frame
	struct command_buffer_system {

		// free not this frame begun command buffers
		std::vector<std::vector<vk::CommandBuffer>> mFreeCommandBuffers;
		// command buffers used this frame, ready to be ended and submitted,
		// not to be used again in this frame, reseted the next time this frame starts
		std::vector<std::vector<vk::CommandBuffer>> mRecordedCommandBuffers;
		// command buffers submitted in this frame
		std::vector<std::vector<vk::CommandBuffer>> mSubmittedCommandBuffers;

		void init_frames(size_t frameCount) {
			mFreeCommandBuffers.resize(frameCount);
			mRecordedCommandBuffers.resize(frameCount);
			mSubmittedCommandBuffers.resize(frameCount);
		}
	};

	class vulkan_command_buffer_manager
	{
	public:
		vulkan_command_buffer_manager(vk::CommandPool & commandPool, vk::Queue &transferQueue);
		vulkan_command_buffer_manager(uint32_t imageCount, vk::CommandPool &commandPool, vk::Queue &transferQueue);
		virtual ~vulkan_command_buffer_manager();

		// TODO better command buffer management, do not always begin command buffer, only if it has not begun yet, etc.
		vk::CommandBuffer get_command_buffer(vk::CommandBufferLevel bufferLevel, vk::CommandBufferBeginInfo &beginInfo);
		std::vector<vk::CommandBuffer> get_recorded_command_buffers(vk::CommandBufferLevel bufferLevel);
		// resets all command buffers at the beginnng of the frame
		void reset_command_buffers();


		vk::CommandBuffer begin_single_time_commands();
		void end_single_time_commands(vk::CommandBuffer commandBuffer);
	private:
		uint32_t mImageCount;

		vk::CommandPool mCommandPool;
		vk::Queue mTransferQueue;

		// deleted with command pool
		command_buffer_system mSecondaryCmdBuffers;
		command_buffer_system mPrimaryCmdBuffers;

		void create_command_buffers();

		vk::CommandBuffer get_or_create_command_buffer(command_buffer_system &cmdBufferSystem, vk::CommandBufferLevel bufferLevel, vk::CommandBufferBeginInfo &beginInfo);
		std::vector<vk::CommandBuffer> get_recorded_command_buffers(command_buffer_system &cmdBufferSystem);
	};

}