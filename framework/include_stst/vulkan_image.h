#pragma once
#include "vulkan_context.h"

#include "vulkan_memory.h"
#include "vulkan_command_buffer_manager.h"
#include "vulkan_buffer.h"

namespace cgb {
	class vulkan_image
	{
	public:
		vulkan_image(std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager, void* pixels, int texWidth, int texHeight, int texChannels);

		vulkan_image(std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager, uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling,
			vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspects);

		virtual ~vulkan_image();

		vk::Image get_image() { return mImage; }
		vk::ImageView get_image_view() { return mImageView; }
		uint32_t get_mip_levels() { return mMipLevels; }

		vk::SampleCountFlagBits get_num_samples() { return mNumSamples; }
		vk::Format get_format() { return mFormat; }
		vk::ImageTiling get_tiling() { return mTiling; }
		vk::ImageUsageFlags get_usage() { return mUsage; }
		vk::MemoryPropertyFlags get_memory_properties() { return mMemoryProperties; }
		vk::ImageAspectFlags get_aspects() { return mAspects; }

		void transition_image_layout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);

		int get_width() { return mTexWidth; }
		int get_height() { return mTexHeight; }
	private:
		vk::Image mImage;
		vulkan_memory mImageMemory;
		vk::ImageView mImageView;

		// properties of image
		int mTexWidth;
		int mTexHeight;
		int mTtexChannels;
		uint32_t mMipLevels;
		vk::SampleCountFlagBits mNumSamples;
		vk::Format mFormat;
		vk::ImageTiling mTiling;
		vk::ImageUsageFlags mUsage;
		vk::MemoryPropertyFlags mMemoryProperties;
		vk::ImageAspectFlags mAspects;


		std::shared_ptr<vulkan_command_buffer_manager> mCommandBufferManager;

		void create_texture_image(void * pixels, int texWidth, int texHeight, int texChannels);
		void create_image(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties, vk::Image & image, vulkan_memory & imageMemory);
		bool has_stencil_component(vk::Format format);
		void copy_buffer_to_image(vulkan_buffer& buffer, vk::Image image, uint32_t width, uint32_t height);
		void generate_mipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

		void create_texture_image_view();
		vk::ImageView create_image_view(vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
	};

}