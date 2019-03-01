#pragma once
#include "vulkan_context.h"

#include "vulkan_memory.h"
#include "vulkan_command_buffer_manager.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"

namespace cgb {

	class vulkan_texture
	{
	public:
		vulkan_texture(vulkan_image * image);
		vulkan_texture(vulkan_image * image, VkSamplerCreateInfo & samplerInfo);
		virtual ~vulkan_texture();

		VkImageView getTextureImageView() { return _image->get_image_view(); };
		VkSampler getTextureSampler() { return _textureSampler; };
	private:
		vulkan_image * _image;

		VkSampler _textureSampler;

		void createTextureSampler();
		void createTextureSampler(VkSamplerCreateInfo &samplerInfo);
	};

}