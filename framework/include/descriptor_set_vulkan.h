#pragma once

namespace cgb // ========================== TODO/WIP =================================
{
	struct descriptor_set
	{
		descriptor_set() noexcept;
		descriptor_set(const vk::DescriptorSet&);
		descriptor_set(const descriptor_set&) = delete;
		descriptor_set(descriptor_set&&) noexcept;
		descriptor_set& operator=(const descriptor_set&) = delete;
		descriptor_set& operator=(descriptor_set&&) noexcept;

		vk::DescriptorSet mDescriptorSet;
	};
}
