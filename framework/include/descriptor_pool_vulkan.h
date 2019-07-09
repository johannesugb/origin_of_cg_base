#pragma once

namespace cgb // ========================== TODO/WIP =================================
{
	struct descriptor_pool
	{
		descriptor_pool() noexcept;
		descriptor_pool(const vk::DescriptorPool&);
		descriptor_pool(const descriptor_pool&) = delete;
		descriptor_pool(descriptor_pool&&) noexcept;
		descriptor_pool& operator=(const descriptor_pool&) = delete;
		descriptor_pool& operator=(descriptor_pool&&) noexcept;
		~descriptor_pool();

		vk::DescriptorPool mDescriptorPool;
	};
}
