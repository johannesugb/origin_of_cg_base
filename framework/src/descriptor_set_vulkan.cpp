namespace cgb
{
	descriptor_set_layout descriptor_set_layout::create(std::initializer_list<binding_data> pBindings)
	{
		std::vector<vk::DescriptorSetLayoutBinding> bindingsVec;
		bindingsVec.reserve(pBindings.size());
		// initializer list -> vector:
		for (auto& b : pBindings) {
			bindingsVec.push_back(b.mLayoutBinding);
		}

		auto createInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(static_cast<uint32_t>(bindingsVec.size()))
			.setPBindings(bindingsVec.data());

		// TODO: Aaaaaaach fuuuuuuck, mitten im WIP =(
		// Weiter geht's mit std::vector<vk::DescriptorSetLayoutBinding> mOrderedBindings;
		//  ^ dazu is noch ein operator <(const vk::DescriptorSetLayoutBinding& left, const vk::DescriptorSetLayoutBinding& right) nötig
		//    ... oder evtl. besser kein extra operator, sondern eine order_in_set(const vk::DescriptorSetLayoutBinding& left, const vk::DescriptorSetLayoutBinding& right) evtl?
		//    ... naja, warum nicht operator < ... aber dann nur operator < und nicht <=, weil kann das gleich sein? weiß nicht...

		return cgb::context().logical_device().createDescriptorSetLayoutUnique(createInfo);
	}

	descriptor_set::descriptor_set() noexcept 
		: mDescriptorSet() 
	{ }

	descriptor_set::descriptor_set(const vk::DescriptorSet& pDescriptorSet)
		: mDescriptorSet(pDescriptorSet)
	{ }

	descriptor_set::descriptor_set(descriptor_set&& other) noexcept
		: mDescriptorSet(std::move(other.mDescriptorSet))
	{ 
		other.mDescriptorSet = nullptr;
	}

	descriptor_set& descriptor_set::operator=(descriptor_set&& other) noexcept
	{ 
		mDescriptorSet = std::move(other.mDescriptorSet);
		other.mDescriptorSet = nullptr;
		return *this;
	}
}
