#include "cg_stdafx.h"
#include "vulkan_resource_bundle.h"

namespace cgb {

	vulkan_resource_bundle::vulkan_resource_bundle(std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, bool dynamicResource) :
		mResourceBundleLayout(resourceBundleLayout), mDynamicResource(dynamicResource)
	{
		size_t resourceCount = get_resource_count();
		mDescriptorWrites.resize(resourceCount);
	}

	vulkan_resource_bundle::~vulkan_resource_bundle()
	{
	}

	void cgb::vulkan_resource_bundle::add_image_resource(uint32_t binding, vk::ImageLayout imageLayout, std::shared_ptr<vulkan_texture> texture, uint32_t dstArrayElement, uint32_t descriptorCount)
	{
		auto descriptorWrites = create_image_resource_descriptor_write(binding, imageLayout, texture, dstArrayElement, descriptorCount);

		size_t resourceCount = get_resource_count();
		for (int i = 0; i < resourceCount; i++) {
			mDescriptorWrites[i].push_back(std::move(descriptorWrites));
		}
		mTextureMap.emplace(binding, std::vector<std::shared_ptr<vulkan_texture>> { texture });
	}

	void cgb::vulkan_resource_bundle::add_dynamic_image_resource(uint32_t binding, vk::ImageLayout imageLayout, std::vector<std::shared_ptr<vulkan_texture>> textures, uint32_t dstArrayElement, uint32_t descriptorCount)
	{
		assert(mDynamicResource);
		assert(textures.size() == vulkan_context::instance().dynamicRessourceCount);

		mDescriptorImageInfoMap[binding].reserve(vulkan_context::instance().dynamicRessourceCount);
		for (int i = 0; i < vulkan_context::instance().dynamicRessourceCount; i++) {
			auto descriptorWrite = create_image_resource_descriptor_write(binding, imageLayout, textures[i], dstArrayElement, descriptorCount);
			mDescriptorWrites[i].push_back(std::move(descriptorWrite));
		}
		mTextureMap.emplace(binding, textures);
	}

	void cgb::vulkan_resource_bundle::add_buffer_resource(uint32_t binding, std::shared_ptr<vulkan_buffer> buffer, vk::DeviceSize range, vk::DeviceSize offset, uint32_t dstArrayElement, uint32_t descriptorCount)
	{
		auto descriptorWrites = create_buffer_resource_descriptor_write(binding, buffer, range, offset, dstArrayElement, descriptorCount);

		size_t resourceCount = get_resource_count();
		for (int i = 0; i < resourceCount; i++) {
			mDescriptorWrites[i].push_back(std::move(descriptorWrites));
		}
		mBufferMap.emplace(binding, std::vector<std::shared_ptr<vulkan_buffer>> { buffer });
	}

	void cgb::vulkan_resource_bundle::add_dynamic_buffer_resource(uint32_t binding, std::vector<std::shared_ptr<vulkan_buffer>> buffers, vk::DeviceSize range, vk::DeviceSize offset, uint32_t dstArrayElement, uint32_t descriptorCount)
	{
		assert(mDynamicResource);
		assert(buffers.size() == vulkan_context::instance().dynamicRessourceCount);

		std::vector<vk::WriteDescriptorSet> descriptorWrites(vulkan_context::instance().dynamicRessourceCount);
		mDescriptorBufferInfoMap[binding].reserve(vulkan_context::instance().dynamicRessourceCount);
		for (int i = 0; i < vulkan_context::instance().dynamicRessourceCount; i++) {
			auto descriptorWrite = create_buffer_resource_descriptor_write(binding, buffers[i], range, offset, dstArrayElement, descriptorCount);
			mDescriptorWrites[i].push_back(descriptorWrite);
		}
		mBufferMap.emplace(binding, buffers);
	}

	vk::WriteDescriptorSet&  cgb::vulkan_resource_bundle::create_image_resource_descriptor_write(uint32_t binding, vk::ImageLayout imageLayout, std::shared_ptr<vulkan_texture> texture, uint32_t dstArrayElement, uint32_t descriptorCount)
	{
		vk::DescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = imageLayout;
		imageInfo.imageView = texture->getTextureImageView();
		imageInfo.sampler = texture->getTextureSampler();

		size_t idx = mDescriptorImageInfoMap[binding].size();
		mDescriptorImageInfoMap[binding].push_back(imageInfo);

		auto bindingDesc = mResourceBundleLayout->get_binding_description(binding);
		vk::WriteDescriptorSet descriptorWrites = {};
		descriptorWrites.dstBinding = binding;
		descriptorWrites.descriptorType = bindingDesc.descriptorType;
		descriptorWrites.dstArrayElement = dstArrayElement;
		descriptorWrites.descriptorCount = descriptorCount;
		descriptorWrites.pImageInfo = &mDescriptorImageInfoMap[binding][idx];
		return descriptorWrites;

	}

	vk::WriteDescriptorSet& cgb::vulkan_resource_bundle::create_buffer_resource_descriptor_write(uint32_t binding, std::shared_ptr<vulkan_buffer> buffer, vk::DeviceSize range, vk::DeviceSize offset, uint32_t dstArrayElement, uint32_t descriptorCount)
	{
		vk::DescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = buffer->get_vk_buffer();
		bufferInfo.offset = offset;
		bufferInfo.range = range;

		size_t idx = mDescriptorBufferInfoMap[binding].size();
		mDescriptorBufferInfoMap[binding].push_back(bufferInfo);

		auto bindingDesc = mResourceBundleLayout->get_binding_description(binding);
		vk::WriteDescriptorSet descriptorWrites = {};
		descriptorWrites.dstBinding = binding;
		descriptorWrites.descriptorType = bindingDesc.descriptorType;
		descriptorWrites.dstArrayElement = dstArrayElement;
		descriptorWrites.descriptorCount = descriptorCount;
		descriptorWrites.pBufferInfo = &mDescriptorBufferInfoMap[binding][idx];

		return descriptorWrites;
	}
}