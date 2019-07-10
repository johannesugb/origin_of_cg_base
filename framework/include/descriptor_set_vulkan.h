#pragma once

namespace cgb // ========================== TODO/WIP =================================
{


	class descriptor_set_layout
	{
	public:
		descriptor_set_layout() = default;
		descriptor_set_layout(const descriptor_set_layout&) = delete;
		descriptor_set_layout(descriptor_set_layout&&) = default;
		descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;
		descriptor_set_layout& operator=(descriptor_set_layout&&) = default;
		~descriptor_set_layout() = default;

		static descriptor_set_layout create(std::initializer_list<binding_data> pBindings);

	private:
		std::vector<vk::DescriptorSetLayoutBinding> mOrderedBindings;
		vk::UniqueDescriptorSetLayout mLayout;
	};
	
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
