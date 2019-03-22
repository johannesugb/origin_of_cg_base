#include "cg_stdafx.h"
#include "vulkan_resource_bundle_layout.h"


namespace cgb {
	vulkan_resource_bundle_layout::vulkan_resource_bundle_layout()
	{
	}

	cgb::vulkan_resource_bundle_layout::vulkan_resource_bundle_layout(vk::DescriptorSetLayout & descriptorSetLayout) : 
		mDescriptorSetLayout(descriptorSetLayout)
	{
		mInitialized = true;
	}

	vulkan_resource_bundle_layout::~vulkan_resource_bundle_layout()
	{
		if (mInitialized) {
			cgb::vulkan_context::instance().device.destroyDescriptorSetLayout(mDescriptorSetLayout);
		}
	}

	// TODO add enum for vk::DescriptorType
	void cgb::vulkan_resource_bundle_layout::add_binding(uint32_t binding, vk::DescriptorType descriptorType, ShaderStageFlagBits shaderStage, uint32_t descriptorCount)
	{
#ifdef _DEBUG
		if (baked()) {
			throw std::runtime_error("resourceBundleLayout already baked. Cannot add more bindings to it!");
		}
#endif
		vk::DescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = descriptorCount;
		layoutBinding.stageFlags = static_cast<vk::ShaderStageFlagBits>(shaderStage);
		layoutBinding.pImmutableSamplers = nullptr; // Optional

		mBindingMap.emplace(binding, layoutBinding);
		mBindings.push_back(layoutBinding);
	}

	void cgb::vulkan_resource_bundle_layout::bake()
	{
		if (!mInitialized) {
			vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.bindingCount = static_cast<uint32_t>(mBindings.size());

			layoutInfo.pBindings = mBindings.data();


			if (cgb::vulkan_context::instance().device.createDescriptorSetLayout(&layoutInfo, nullptr, &mDescriptorSetLayout) != vk::Result::eSuccess) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
		}
		mInitialized = true;
	}
}