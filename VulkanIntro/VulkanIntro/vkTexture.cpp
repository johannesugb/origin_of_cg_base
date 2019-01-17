#include "vkTexture.h"

#include <algorithm>
#include <stdexcept>

#include "vkCgbBuffer.h"
#include "vkMemoryManager.h"


vkTexture::vkTexture(vkCgbImage * image) : _image(image)
{
	createTextureSampler();
}

vkTexture::vkTexture(vkCgbImage * image, VkSamplerCreateInfo &samplerInfo) : _image(image)
{
	createTextureSampler(samplerInfo);
}


vkTexture::~vkTexture()
{
	vkDestroySampler(vkContext::instance().vkContext::instance().device, _textureSampler, nullptr);
}

void vkTexture::createTextureSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(_image->get_mip_levels());

	createTextureSampler(samplerInfo);
}

void vkTexture::createTextureSampler(VkSamplerCreateInfo &samplerInfo) {

	if (vkCreateSampler(vkContext::instance().device, &samplerInfo, nullptr, &_textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}