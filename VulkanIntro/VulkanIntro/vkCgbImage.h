#pragma once
#include "vkContext.h"

#include "vkCgbMemory.h"
#include "vkCommandBufferManager.h"
#include "vkCgbBuffer.h"

class vkCgbImage
{
public:
	vkCgbImage(vkCommandBufferManager* commandBufferManager, void* pixels, int texWidth, int texHeight, int texChannels);

	vkCgbImage(vkCommandBufferManager * commandBufferManager, uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspects);

	virtual ~vkCgbImage();

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
	vkCgbMemory mImageMemory;
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


	vkCommandBufferManager* mCommandBufferManager;

	void create_texture_image(void * pixels, int texWidth, int texHeight, int texChannels);
	void create_image(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties, vk::Image & image, vkCgbMemory & imageMemory);
	bool has_stencil_component(vk::Format format);
	void copy_buffer_to_image(vkCgbBuffer& buffer, vk::Image image, uint32_t width, uint32_t height);
	void generate_mipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	void create_texture_image_view();
	vk::ImageView create_image_view(vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
};

