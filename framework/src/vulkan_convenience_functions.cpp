#include "vulkan_convenience_functions.h"

namespace cgb
{
	vk::IndexType to_vk_index_type(size_t pSize)
	{
		if (pSize == sizeof(uint16_t)) {
			return vk::IndexType::eUint16;
		}
		if (pSize == sizeof(uint32_t)) {
			return vk::IndexType::eUint32;
		}
		LOG_ERROR(fmt::format("The given size[{}] does not correspond to a valid vk::IndexType", pSize));
		return vk::IndexType::eNoneNV;
	}

	vk::ImageViewType to_image_view_type(const vk::ImageCreateInfo& info)
	{
		switch (info.imageType)
		{
		case vk::ImageType::e1D:
			if (info.arrayLayers > 1) {
				return vk::ImageViewType::e1DArray;
			}
			else {
				return vk::ImageViewType::e1D;
			}
		case vk::ImageType::e2D:
			if (info.arrayLayers > 1) {
				return vk::ImageViewType::e2DArray;
			}
			else {
				return vk::ImageViewType::e2D;
			}
		case vk::ImageType::e3D:
			return vk::ImageViewType::e3D;
		}
		throw new std::runtime_error("It might be that the implementation of to_image_view_type(const vk::ImageCreateInfo& info) is incomplete. Please complete it!");
	}
}
