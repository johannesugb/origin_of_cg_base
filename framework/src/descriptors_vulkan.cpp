#include "descriptors_vulkan.h"

namespace cgb
{

	vk::UniqueDescriptorSetLayout layout_for(std::initializer_list<binding_data> pBindings)
	{
		std::vector<vk::DescriptorSetLayoutBinding> bindingsVec;
		bindingsVec.reserve(pBindings.size());
		// initializer list -> vector:
		std::transform(std::begin(pBindings), std::end(pBindings), std::back_inserter(bindingsVec), [](binding_data& element) { return element.mLayoutBinding; });

		auto createInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(static_cast<uint32_t>(bindingsVec.size()))
			.setPBindings(bindingsVec.data());
		return cgb::context().logical_device().createDescriptorSetLayoutUnique(createInfo);
	}
}
