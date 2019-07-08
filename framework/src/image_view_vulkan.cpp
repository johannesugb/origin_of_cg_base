#include "image_view_vulkan.h"

namespace cgb
{
	image_view_t image_view_t::create(cgb::image pImageToOwn, image_format pViewFormat, context_specific_function<void(image_view_t&)> pAlterConfigBeforeCreation)
	{
		image_view_t result;
		
		// Transfer ownership:
		result.mImage = std::move(pImageToOwn);
		
		// Config for the image view:
		vk::ImageAspectFlags imageAspectFlags;
		{
			// Guess the vk::ImageAspectFlags:
			auto imageFormat = image_format(result.image().config().format);
			if (is_depth_format(imageFormat)) {
				imageAspectFlags |= vk::ImageAspectFlagBits::eDepth;
				if (has_stencil_component(imageFormat)) {
					imageAspectFlags |= vk::ImageAspectFlagBits::eStencil;
				}
			}
			else {
				imageAspectFlags |= vk::ImageAspectFlagBits::eColor;
			}
			// vk::ImageAspectFlags handling is probably incomplete => Use pAlterConfigBeforeCreation to adapt the config to your requirements!
		}
		// Proceed with config creation (and use the imageAspectFlags there):
		result.mInfo = vk::ImageViewCreateInfo()
			.setImage(result.image_handle())
			.setViewType(to_image_view_type(result.image().config()))
			.setFormat(pViewFormat.mFormat)
			.setSubresourceRange(vk::ImageSubresourceRange()
				.setAspectMask(imageAspectFlags)
				.setBaseMipLevel(0u)
				.setLevelCount(result.image().config().mipLevels)
				.setBaseArrayLayer(0u)
				.setLayerCount(result.image().config().arrayLayers));

		// Maybe alter the config?!
		if (pAlterConfigBeforeCreation.mFunction) {
			pAlterConfigBeforeCreation.mFunction(result);
		}

		result.mImageView = context().logical_device().createImageViewUnique(result.mInfo);
	}
}
