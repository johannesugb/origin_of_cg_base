#pragma once

namespace cgb
{
	/**	Represents a descriptor set layout, contains also the bindings
	 *	stored in binding-order, and the accumulated descriptor counts
	 *	per bindings which is information that can be used for configuring
	 *	descriptor pools.
	 */
	class descriptor_set_layout
	{
	public:
		descriptor_set_layout() = default;
		descriptor_set_layout(const descriptor_set_layout&) = delete;
		descriptor_set_layout(descriptor_set_layout&&) = default;
		descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;
		descriptor_set_layout& operator=(descriptor_set_layout&&) = default;
		~descriptor_set_layout() = default;

		auto& required_pool_sizes() const { return mBindingRequirements; }
		auto& bindings() const { return mOrderedBindings; }
		auto handle() const { return mLayout.get(); }

		static descriptor_set_layout create(std::initializer_list<binding_data> pBindings);

	private:
		std::vector<vk::DescriptorPoolSize> mBindingRequirements;
		std::vector<vk::DescriptorSetLayoutBinding> mOrderedBindings;
		vk::UniqueDescriptorSetLayout mLayout;
	};
	
	/** Descriptor set */
	class descriptor_set
	{
	public:
		descriptor_set() = default;
		descriptor_set(const descriptor_set&) = delete;
		descriptor_set(descriptor_set&&) = default;
		descriptor_set& operator=(const descriptor_set&) = delete;
		descriptor_set& operator=(descriptor_set&&) = default;
		~descriptor_set() = default;

		static descriptor_set create(std::initializer_list<binding_data> pBindings);

	private:
		vk::UniqueDescriptorSet mDescriptorSet;
	};
}
