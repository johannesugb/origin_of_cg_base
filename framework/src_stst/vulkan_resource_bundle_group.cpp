#include "cg_stdafx.h"
#include "vulkan_resource_bundle_group.h"

namespace cgb {

	vulkan_resource_bundle_group::vulkan_resource_bundle_group()
	{
		mDescriptorSetCount = 0;
	}


	vulkan_resource_bundle_group::~vulkan_resource_bundle_group()
	{
		if (mAllocationStarted) {
			cgb::vulkan_context::instance().device.destroyDescriptorPool(mDescriptorPool);
		}
	}

	std::shared_ptr<vulkan_resource_bundle> cgb::vulkan_resource_bundle_group::create_resource_bundle(
		std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, bool dynamicResource)
	{
		// std::make_shared not possible due to friend private constructor
		auto resourceBundle = std::shared_ptr<vulkan_resource_bundle>(new vulkan_resource_bundle(resourceBundleLayout, dynamicResource));

		auto resourceCount = resourceBundle->get_resource_count();

		for (auto binding : resourceBundleLayout->get_bindings()) {
			if (mCountPerType.find(binding.descriptorType) == mCountPerType.end()) {
				mCountPerType[binding.descriptorType] = 0;
			}
			mCountPerType[binding.descriptorType] += binding.descriptorCount * resourceCount;
		}

		mDescriptorSetCount += resourceCount;
		return resourceBundle;
	}

	void cgb::vulkan_resource_bundle_group::allocate_resource_bundle(vulkan_resource_bundle* resourceBundle)
	{
		if (!mAllocationStarted) {
			create_descriptor_pool();
			mAllocationStarted = true;
		}

		size_t resourceCount = resourceBundle->get_resource_count();

		std::vector<vk::DescriptorSetLayout> layouts(resourceCount, resourceBundle->mResourceBundleLayout->get_descriptor_set_layout());
		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(resourceCount);
		allocInfo.pSetLayouts = layouts.data();

		resourceBundle->mDescriptorSets.resize(resourceCount);
		if (vulkan_context::instance().device.allocateDescriptorSets(&allocInfo, resourceBundle->mDescriptorSets.data()) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < resourceCount; i++) {

			for (auto& descriptorWrite : resourceBundle->mDescriptorWrites[i]) {
				descriptorWrite.dstSet = resourceBundle->mDescriptorSets[i];
			}
			vulkan_context::instance().device.updateDescriptorSets(static_cast<uint32_t>(resourceBundle->mDescriptorWrites[i].size()), resourceBundle->mDescriptorWrites[i].data(), 0, nullptr);
		}
	}

	void cgb::vulkan_resource_bundle_group::create_descriptor_pool()
	{
		std::vector<vk::DescriptorPoolSize> poolSizes(mCountPerType.size());

		std::transform(mCountPerType.begin(), mCountPerType.end(), poolSizes.begin(), [] (auto countPerTypePair)
		{
			vk::DescriptorPoolSize poolSize;
			poolSize.type = countPerTypePair.first;
			poolSize.descriptorCount = countPerTypePair.second;

			return poolSize;
		});

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(mDescriptorSetCount);

		if (cgb::vulkan_context::instance().device.createDescriptorPool(&poolInfo, nullptr, &mDescriptorPool) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

}