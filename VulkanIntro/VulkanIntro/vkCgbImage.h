#pragma once
#include "vkContext.h"

#include "vkCgbMemory.h"
#include "vkCommandBufferManager.h"
#include "vkCgbBuffer.h"

class vkCgbImage
{
public:
	vkCgbImage(vkCommandBufferManager* commandBufferManager, void* pixels, int texWidth, int texHeight, int texChannels);
	virtual ~vkCgbImage();

	VkImageView getImageView() { return _imageView; };
	uint32_t getMipLevels() { return _mipLevels; };
private:
	int _texWidth;
	int _texHeight;
	int _texChannels;

	VkImage _image;
	vkCgbMemory _imageMemory;
	VkImageView _imageView;

	uint32_t _mipLevels;

	vkCommandBufferManager* _commandBufferManager;

	void createTextureImage(void * pixels, int texWidth, int texHeight, int texChannels);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage & image, vkCgbMemory & imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	bool hasStencilComponent(VkFormat format);
	void copyBufferToImage(vkCgbBuffer& buffer, VkImage image, uint32_t width, uint32_t height);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	void createTextureImageView();
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
};

