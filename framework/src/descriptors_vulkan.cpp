#include "descriptors_vulkan.h"

namespace cgb
{

	vk::UniqueDescriptorSetLayout layout_for(std::initializer_list<binding_data> pBindings)
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
		return cgb::context().logical_device().createDescriptorSetLayoutUnique(createInfo);
	}
}
