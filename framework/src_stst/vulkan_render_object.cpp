#include "vulkan_render_object.h"

namespace cgb {

	vulkan_render_object::vulkan_render_object(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<std::shared_ptr<vulkan_resource_bundle>> resourceBundles) 
		: mVertices(vertices), mIndices(indices), mIndexCount(indices.size()) {
		mPushConstData = std::make_shared<PushUniforms>();
		mResourceBundles.insert(mResourceBundles.end(), resourceBundles.begin(), resourceBundles.end());
		create_vertex_index_buffer();
	}

	vulkan_render_object::vulkan_render_object(std::vector<std::shared_ptr<vulkan_buffer>> vertexBuffers, std::shared_ptr<vulkan_buffer> indexBuffer, size_t indexCount,
		std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, std::shared_ptr<vulkan_resource_bundle_group> resourceBundleGroup,
		std::vector<std::shared_ptr<vulkan_resource_bundle>> resourceBundles) :
	mVertexBuffers(vertexBuffers), mIndexBuffer(indexBuffer), mIndexCount(indexCount)
	{
		mPushConstData = std::make_shared<PushUniforms>();
		create_uniform_buffer();
		mResourceBundles.insert(mResourceBundles.end(), resourceBundles.begin(), resourceBundles.end());
		create_descriptor_sets(resourceBundleLayout, resourceBundleGroup, nullptr, {});
	}

	vulkan_render_object::vulkan_render_object(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
		std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, std::shared_ptr<vulkan_resource_bundle_group> resourceBundleGroup,
		std::shared_ptr<vulkan_texture> texture, std::vector<std::shared_ptr<vulkan_texture>> debugTextures)
		:  mVertices(vertices), mIndices(indices), mIndexCount(indices.size())
	{
		mPushConstData = std::make_shared<PushUniforms>();
		create_vertex_index_buffer();

		create_uniform_buffer();
		create_descriptor_sets(resourceBundleLayout, resourceBundleGroup, texture, debugTextures);
	}


	vulkan_render_object::~vulkan_render_object()
	{
	}

	void vulkan_render_object::create_uniform_buffer() {
		vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

		mUniformBuffers.resize(vulkan_context::instance().dynamicRessourceCount);

		for (size_t i = 0; i < vulkan_context::instance().dynamicRessourceCount; i++) {
			mUniformBuffers[i] = std::make_shared<vulkan_buffer>(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		}
	}

	void vulkan_render_object::create_descriptor_sets(std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, std::shared_ptr<vulkan_resource_bundle_group> resourceBundleGroup, std::shared_ptr<vulkan_texture> texture, std::vector<std::shared_ptr<vulkan_texture>> debugTextures) {
		
		auto resourceBundle = resourceBundleGroup->create_resource_bundle(resourceBundleLayout, true);
		resourceBundle->add_dynamic_buffer_resource(0, mUniformBuffers, sizeof(UniformBufferObject));

		if (texture) {
			resourceBundle->add_image_resource(1, vk::ImageLayout::eShaderReadOnlyOptimal, texture);
		}

		if (vulkan_context::instance().shadingRateImageSupported && !debugTextures.empty()) {
			resourceBundle->add_dynamic_image_resource(2, vk::ImageLayout::eShaderReadOnlyOptimal, debugTextures);
		}

		mResourceBundles.push_back(resourceBundle);
	}

	void vulkan_render_object::update_uniform_buffer(uint32_t currentImage, UniformBufferObject ubo) {
		if (mUniformBuffers.size() > 0) {
			mUniformBuffers[currentImage]->update_buffer(&ubo, sizeof(ubo));
		}
	}

	void cgb::vulkan_render_object::update_push_constant(std::shared_ptr<void> pushConstData)
	{
		mPushConstData = pushConstData;
	}

	void vulkan_render_object::create_vertex_index_buffer() {
		auto vertexBuffer = std::make_shared<vulkan_buffer>(sizeof(mVertices[0]) * mVertices.size(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, mVertices.data());
		mVertexBuffers.push_back(vertexBuffer);

		mIndexBuffer = std::make_shared<vulkan_buffer>(sizeof(mIndices[0]) * mIndices.size(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, mIndices.data());
	}
}