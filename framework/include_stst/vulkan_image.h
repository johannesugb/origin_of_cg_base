#pragma once
#include "vulkan_context.h"

#include "vulkan_memory.h"
#include "vulkan_command_buffer_manager.h"
#include "vulkan_buffer.h"

namespace cgb {
	class vulkan_image
	{
	public:
		vulkan_image(vk::Image image, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t texChannels, vk::SampleCountFlagBits numSamples, vk::Format format,
			vk::ImageUsageFlags usage, vk::ImageAspectFlags aspects, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager = vulkan_context::instance().transferCommandBufferManager);

		vulkan_image(vk::Image image, vk::ImageView imageView, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t texChannels, vk::SampleCountFlagBits numSamples, vk::Format format, 
			vk::ImageUsageFlags usage, vk::ImageAspectFlags aspects, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager = vulkan_context::instance().transferCommandBufferManager);

		vulkan_image(void* pixels, uint32_t texWidth, uint32_t texHeight, int texChannels, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager = vulkan_context::instance().transferCommandBufferManager);

		vulkan_image(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t texChannels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling,
			vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspects, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager = vulkan_context::instance().transferCommandBufferManager);

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
		void update_pixels(void* pixels);

		int get_width() { return mTexWidth; }
		int get_height() { return mTexHeight; }

		void generate_mipmaps(vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

		static std::shared_ptr<vulkan_image> generate_1px_image(uint8_t color_r, uint8_t color_g, uint8_t color_b);
		static void blit_image(vulkan_image* srcImage, vulkan_image* targetImage,
			vk::ImageLayout srcInitLayout, vk::ImageLayout srcEndLayout, vk::ImageLayout targetInitLayout, vk::ImageLayout targetEndLayout,
			vk::AccessFlagBits srcInitAccFlags, vk::AccessFlagBits srcEndAccFlags, vk::AccessFlagBits targetInitAccFlags, vk::AccessFlagBits targetEndAccFlags,
			vk::PipelineStageFlagBits srcStage, vk::PipelineStageFlagBits targetStage,
			vulkan_command_buffer_manager* commandBufferManager);
	private:
		vk::Image mImage;
		vulkan_memory mImageMemory;
		vk::ImageView mImageView;

		// properties of image
		uint32_t mTexWidth;
		uint32_t mTexHeight;
		uint32_t mTtexChannels;
		uint32_t mMipLevels;
		vk::SampleCountFlagBits mNumSamples;
		vk::Format mFormat;
		vk::ImageTiling mTiling;
		vk::ImageUsageFlags mUsage;
		vk::MemoryPropertyFlags mMemoryProperties;
		vk::ImageAspectFlags mAspects;


		std::shared_ptr<vulkan_command_buffer_manager> mCommandBufferManager;

		bool vkImageViewNotControlled = false;
		bool vkImageNotControlled = false;

		void create_texture_image(void * pixels, uint32_t texWidth, uint32_t texHeight, int texChannels);
		void create_image(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties, vk::Image & image, vulkan_memory & imageMemory);
		bool has_stencil_component(vk::Format format);
		void copy_buffer_to_image(vulkan_buffer& buffer, vk::Image image, uint32_t width, uint32_t height);

		void create_texture_image_view();
		vk::ImageView create_image_view(vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
	};

}