namespace cgb
{
	descriptor_set_layout descriptor_set_layout::create(std::initializer_list<binding_data> pBindings)
	{
		// Put elements from initializer list into vector of ORDERED bindings:
		descriptor_set_layout result;
		result.mBindingRequirements.reserve(pBindings.size());
		result.mOrderedBindings.reserve(pBindings.size());
		for (auto& b : pBindings) {
			// find position where to insert in vector
			{
				// ordered by descriptor type
				auto entry = vk::DescriptorPoolSize{}
					.setType(b.mLayoutBinding.descriptorType)
					.setDescriptorCount(b.mLayoutBinding.descriptorCount);
				auto it = std::lower_bound(std::begin(result.mBindingRequirements), std::end(result.mBindingRequirements), 
					entry,
					[](const vk::DescriptorPoolSize& first, const vk::DescriptorPoolSize& second) -> bool {
						using EnumType = std::underlying_type<vk::DescriptorType>::type;
						return static_cast<EnumType>(first.type) < static_cast<EnumType>(second.type);
					});
				// Maybe accumulate
				if (it != std::end(result.mBindingRequirements) && it->type == entry.type) {
					it->descriptorCount += entry.descriptorCount;
				}
				else {
					result.mBindingRequirements.insert(it, entry);
				}
			}
			{
				// ordered by binding
				auto it = std::lower_bound(std::begin(result.mOrderedBindings), std::end(result.mOrderedBindings), 
					b.mLayoutBinding,
					[](const vk::DescriptorSetLayoutBinding& first, const vk::DescriptorSetLayoutBinding& second) -> bool {
						assert(first.binding != second.binding);
						return first.binding < second.binding;
					});
				result.mOrderedBindings.insert(it, b.mLayoutBinding);
			}
		}

		// Allocate the layout and return the result:
		auto createInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(static_cast<uint32_t>(result.mOrderedBindings.size()))
			.setPBindings(result.mOrderedBindings.data());
		result.mLayout = cgb::context().logical_device().createDescriptorSetLayoutUnique(createInfo);
		return result;
	}




	descriptor_set descriptor_set::create(std::initializer_list<binding_data> pBindings)
	{
	}
}
