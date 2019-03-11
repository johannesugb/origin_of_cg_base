#pragma once

#include <unordered_map>

#include "vulkan_context.h"

#include "vulkan_resource_bundle_layout.h"
#include "vulkan_resource_bundle.h"

namespace cgb {

	class vulkan_resource_bundle_group
	{
	public:
		vulkan_resource_bundle_group();
		virtual ~vulkan_resource_bundle_group();

		std::shared_ptr<vulkan_resource_bundle> create_resource_bundle(std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, bool dynamicResource);

		void allocate_resource_bundle(vulkan_resource_bundle* resourceBundle);
	private:
		// if allocation has been started no ResouceBundles (DescriptorSets) can be created from this group
		bool mAllocationStarted = false;

		// saves the count per descriptor type for DescriptorPool creation
		std::unordered_map<vk::DescriptorType, uint32_t> mCountPerType;
		uint32_t mDescriptorSetCount;

		vk::DescriptorPool mDescriptorPool;

		void create_descriptor_pool();
	};

}