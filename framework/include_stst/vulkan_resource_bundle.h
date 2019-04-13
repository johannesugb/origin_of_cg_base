#pragma once

#include <unordered_map>

#include "vulkan_resource_bundle_layout.h"
#include "vulkan_texture.h"
#include "vulkan_buffer.h"

namespace cgb {

	class vulkan_resource_bundle_group;

	class vulkan_resource_bundle
	{
		friend class vulkan_resource_bundle_group;

	public:
		virtual ~vulkan_resource_bundle();

		void add_image_resource(uint32_t binding, vk::ImageLayout imageLayout, std::shared_ptr<vulkan_texture> texture, 
			uint32_t dstArrayElement = 0, uint32_t descriptorCount = 1);

		void add_dynamic_image_resource(uint32_t binding, vk::ImageLayout imageLayout, std::vector<std::shared_ptr<vulkan_texture>> textures,
			uint32_t dstArrayElement = 0, uint32_t descriptorCount = 1);

		void add_image_resource(uint32_t binding, vk::ImageLayout imageLayout, std::shared_ptr<vulkan_image> texture,
			uint32_t dstArrayElement = 0, uint32_t descriptorCount = 1);

		void add_dynamic_image_resource(uint32_t binding, vk::ImageLayout imageLayout, std::vector<std::shared_ptr<vulkan_image>> textures,
			uint32_t dstArrayElement = 0, uint32_t descriptorCount = 1);


		void add_buffer_resource(uint32_t binding, std::shared_ptr<vulkan_buffer> buffer, vk::DeviceSize range, vk::DeviceSize offset = 0, uint32_t dstArrayElement = 0, uint32_t descriptorCount = 1);

		void add_dynamic_buffer_resource(uint32_t binding, std::vector<std::shared_ptr<vulkan_buffer>> buffers, vk::DeviceSize range, vk::DeviceSize offset = 0, uint32_t dstArrayElement = 0, uint32_t descriptorCount = 1);


		std::vector<vk::DescriptorSet> get_descriptor_sets() { return mDescriptorSets; }
		vk::DescriptorSet& get_descriptor_set() { return mDynamicResource ? mDescriptorSets[vulkan_context::instance().currentFrame] : mDescriptorSets[0]; }

	private:
		vulkan_resource_bundle(std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, bool dynamicResource);

		std::shared_ptr<vulkan_resource_bundle_layout> mResourceBundleLayout;
		std::vector<vk::DescriptorSet> mDescriptorSets;

		// TODO maybe clear after descriptor set creation
		std::vector<std::vector<vk::WriteDescriptorSet>> mDescriptorWrites;

		std::unordered_map<uint32_t, std::vector<std::shared_ptr<vulkan_texture>>> mTextureMap;
		std::unordered_map<uint32_t, std::vector<std::shared_ptr<vulkan_image>>> mImageMap;
		std::unordered_map<uint32_t, std::vector<std::shared_ptr<vulkan_buffer>>> mBufferMap;
		std::unordered_map<uint32_t, std::vector<vk::DescriptorImageInfo>> mDescriptorImageInfoMap;
		std::unordered_map<uint32_t, std::vector<vk::DescriptorBufferInfo>> mDescriptorBufferInfoMap;

		vk::WriteDescriptorSet create_image_resource_descriptor_write(uint32_t binding, vk::ImageLayout imageLayout, std::shared_ptr<vulkan_texture> texture,
			uint32_t dstArrayElement = 0, uint32_t descriptorCount = 1);
		vk::WriteDescriptorSet create_image_resource_descriptor_write(uint32_t binding, vk::ImageLayout imageLayout, std::shared_ptr<vulkan_image> image,
			uint32_t dstArrayElement = 0, uint32_t descriptorCount = 1);

		vk::WriteDescriptorSet create_buffer_resource_descriptor_write(uint32_t binding, std::shared_ptr<vulkan_buffer> buffer, vk::DeviceSize range, vk::DeviceSize offset = 0, uint32_t dstArrayElement = 0, uint32_t descriptorCount = 1);


		// specifies if this resource bundle contains dynamic resources
		// e.g. resources which are updated on a per frame basis
		bool mDynamicResource; 

		size_t get_resource_count() { return (mDynamicResource) ? vulkan_context::instance().dynamicRessourceCount : 1; };

	};

}