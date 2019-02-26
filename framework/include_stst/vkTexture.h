#pragma once
#include "vkContext.h"

#include "vkCgbMemory.h"
#include "vkCommandBufferManager.h"
#include "vkCgbBuffer.h"
#include "vkCgbImage.h"

class vkTexture
{
public:
	vkTexture(vkCgbImage * image);
	vkTexture(vkCgbImage * image, VkSamplerCreateInfo & samplerInfo);
	virtual ~vkTexture();

	VkImageView getTextureImageView() { return _image->get_image_view(); };
	VkSampler getTextureSampler() { return _textureSampler; };
private:
	vkCgbImage * _image;

	VkSampler _textureSampler;

	void createTextureSampler();
	void createTextureSampler(VkSamplerCreateInfo &samplerInfo);
};

