#pragma once

namespace cgb
{
	template <typename V, typename F>
	bool has_flag(V value, F flag)
	{
		return (value & flag) == flag;
	}

	extern vk::IndexType to_vk_index_type(size_t pSize);

	extern vk::ImageViewType to_image_view_type(const vk::ImageCreateInfo& info);
}
