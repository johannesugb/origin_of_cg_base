#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <array>

#include "vulkan_context.h"
#include "vulkan_buffer.h"
#include "vulkan_texture.h"
#include "vulkan_resource_bundle_layout.h"
#include "vulkan_resource_bundle_group.h"
#include "vulkan_resource_bundle.h"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

const std::vector<Vertex> verticesQuad = {
	{ { -0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
{ { 0.5f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
{ { 0.5f, 0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
{ { -0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },

{ { -0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
{ { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
{ { 0.5f, 0.5f, -0.5f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
{ { -0.5f, 0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } }
};

const std::vector<uint32_t> indicesQuad = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};

const std::vector<Vertex> verticesScreenQuad = {
	{ { -1.0f, -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
{ { 1.0f, -1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },
{ { 1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
{ { -1.0f, 1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } }
};

const std::vector<uint32_t> indicesScreenQuad = {
	0, 2, 1, 2, 0, 3
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 mvp;
	glm::mat4 mv;
	glm::vec2 frameOffset;
};

struct PushUniforms {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 mvp;
};

namespace cgb {

	class vulkan_render_object
	{
	public:
		// simple standard constructor, ATTENTION no resource (uniform buffers, textures, etc.) management
		// do not call any of these methods (e.g. update_uniform_buffer)
		vulkan_render_object(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<std::shared_ptr<vulkan_resource_bundle>> resourceBundles);

		vulkan_render_object(std::vector<std::shared_ptr<vulkan_buffer>> vertexBuffers, std::shared_ptr<vulkan_buffer> indexBuffer, size_t indexCount,
			std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, std::shared_ptr<vulkan_resource_bundle_group> resourceBundleGroup,
			std::vector<std::shared_ptr<vulkan_resource_bundle>> resourceBundles);

		vulkan_render_object(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
			std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, std::shared_ptr<vulkan_resource_bundle_group> resourceBundleGroup,
			std::shared_ptr<vulkan_texture> texture, std::vector<std::shared_ptr<vulkan_texture>> debugTextures);
		virtual ~vulkan_render_object();

		size_t get_index_count() { return mIndexCount; }
		std::vector<Vertex> get_vertices() { return mVertices; }
		std::vector<uint32_t> get_indices() { return mIndices; }

		vk::Buffer get_vertex_buffer(int i) { return mVertexBuffers[i]->get_vk_buffer(); }
		//void setVertexBuffer(vk::Buffer vertexBuffer) { _vertexBuffer = vertexBuffer; }
		vk::Buffer get_index_buffer() { return mIndexBuffer->get_vk_buffer(); }
		//void setIndexBuffer(vk::Buffer indexBuffer) { _indexBuffer = indexBuffer; }

		void* get_push_uniforms() { return mPushConstData.get(); }

		std::vector<std::shared_ptr<vulkan_resource_bundle>> get_resource_bundles() { return mResourceBundles; }

		void update_uniform_buffer(uint32_t currentImage, UniformBufferObject ubo);
		void update_push_constant(std::shared_ptr<void> pushConstData);

	private:
		std::vector<Vertex> mVertices;
		std::vector<uint32_t> mIndices;

		std::vector<std::shared_ptr<vulkan_buffer>> mVertexBuffers;
		std::shared_ptr<vulkan_buffer> mIndexBuffer; 
		size_t mIndexCount;

		std::vector<std::shared_ptr<vulkan_buffer>> mUniformBuffers;

		std::shared_ptr<void> mPushConstData;
		// TODO PERFORMANCE use multiple descriptor sets / per render pass, e.g. shadow pass does not use textures
		// or like suggested for AMD Hardware, use one large descriptor set for everything and index into Texture arrays and uniforms
		// suggestion: use an array of descriptor sets, the drawer then can decide, which descriptor set to use, 
		// additionally allow global/per drawer descriptor sets
		// this offers the most flexibility for the user of the framework, while still being easy to use
		std::vector<std::shared_ptr<vulkan_resource_bundle>> mResourceBundles;

		void create_uniform_buffer();
		void create_descriptor_sets(std::shared_ptr<vulkan_resource_bundle_layout> resourceBundleLayout, std::shared_ptr<vulkan_resource_bundle_group> resourceBundleGroup,
			std::shared_ptr<vulkan_texture> texture, std::vector<std::shared_ptr<vulkan_texture>> debugTextures);

		void create_vertex_index_buffer();
	};

}