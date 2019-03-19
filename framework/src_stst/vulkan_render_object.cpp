#include "vulkan_render_object.h"

namespace cgb {

	vulkan_render_object::vulkan_render_object(std::vector<std::shared_ptr<vulkan_buffer>> vertexBuffers, std::shared_ptr<vulkan_buffer> indexBuffer, size_t indexCount) :
	mVertexBuffers(vertexBuffers), mIndexBuffer(indexBuffer), mIndexCount(indexCount) {

	}

	vulkan_render_object::vulkan_render_object(uint32_t imageCount, std::vector<Vertex> vertices, std::vector<uint32_t> indices,
		std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, std::shared_ptr<vulkan_resource_bundle_group> resourceBundleGroup,
		std::shared_ptr<vulkan_texture> texture, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager, std::vector<std::shared_ptr<vulkan_texture>> debugTextures)
		: mImageCount(imageCount), mVertices(vertices), mIndices(indices), mIndexCount(indices.size())
	{
		auto vertexBuffer = std::make_shared<vulkan_buffer>(sizeof(mVertices[0]) * mVertices.size(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, mVertices.data());
		mVertexBuffers.push_back(vertexBuffer);

		mIndexBuffer = std::make_shared<vulkan_buffer>(sizeof(mIndices[0]) * mIndices.size(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, mIndices.data());


		create_uniform_buffer(commandBufferManager);
		create_descriptor_sets(resourceBundleLayout, resourceBundleGroup, texture, debugTextures);
	}


	vulkan_render_object::~vulkan_render_object()
	{
	}

	void vulkan_render_object::create_uniform_buffer(std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager) {
		vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

		mUniformBuffers.resize(mImageCount);

		for (size_t i = 0; i < mImageCount; i++) {
			mUniformBuffers[i] = std::make_shared<vulkan_buffer>(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		}
	}

	void vulkan_render_object::create_descriptor_sets(std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, std::shared_ptr<vulkan_resource_bundle_group> resourceBundleGroup, std::shared_ptr<vulkan_texture> texture, std::vector<std::shared_ptr<vulkan_texture>> debugTextures) {
		
		mResourceBundle = resourceBundleGroup->create_resource_bundle(resourceBundleLayout, true);
		mResourceBundle->add_dynamic_buffer_resource(0, mUniformBuffers, sizeof(UniformBufferObject));
		mResourceBundle->add_image_resource(1, vk::ImageLayout::eShaderReadOnlyOptimal, texture);

		if (vulkan_context::instance().shadingRateImageSupported) {
			mResourceBundle->add_dynamic_image_resource(2, vk::ImageLayout::eShaderReadOnlyOptimal, debugTextures);
		}
	}

	void vulkan_render_object::update_uniform_buffer(uint32_t currentImage, UniformBufferObject ubo) {

		mPushUniforms.model = ubo.model;
		mPushUniforms.view = ubo.view;
		mPushUniforms.proj = ubo.proj;
		mPushUniforms.mvp = ubo.mvp;

		mUniformBuffers[currentImage]->update_buffer(&ubo, sizeof(ubo));
	}

}