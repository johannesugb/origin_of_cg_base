#pragma once

#include <unordered_map>

#include "vulkan_context.h"


namespace cgb {
	class vulkan_resource_bundle_layout
	{
	public:
		vulkan_resource_bundle_layout();
		vulkan_resource_bundle_layout(vk::DescriptorSetLayout& descriptorSetLayout);

		virtual ~vulkan_resource_bundle_layout();

		vk::DescriptorSetLayout& get_descriptor_set_layout() { return mDescriptorSetLayout; }

		void bake();

		void add_binding(uint32_t binding, vk::DescriptorType descriptorType, ShaderStageFlagBits shaderStage, uint32_t descriptorCount = 1);
		vk::DescriptorSetLayoutBinding& get_binding_description(uint32_t binding) { return mBindingMap[binding]; };

		std::vector<vk::DescriptorSetLayoutBinding>& get_bindings() { return mBindings; }

	private:
		bool mInitialized = false;
		vk::DescriptorSetLayout mDescriptorSetLayout;

		// key value map for each binding, key is the binding, value the description of the binding
		std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> mBindingMap;
		std::vector<vk::DescriptorSetLayoutBinding> mBindings;
	};
}
