#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <array>

#include "vkContext.h"
#include "vkCgbBuffer.h"
#include "vkTexture.h"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static vk::VertexInputBindingDescription getBindingDescription() {
		vk::VertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 1;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;
		return bindingDescription;
	}

	static vk::VertexInputBindingDescription getBindingDescription2() {
		vk::VertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 2;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;
		return bindingDescription;
	}

	static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {};
		attributeDescriptions[0].binding = 1;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 2;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 2;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

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
	{ { -1.0f, -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
{ { 1.0f, -1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
{ { 1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
{ { -1.0f, 1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } }
};

const std::vector<uint32_t> indicesScreenQuad = {
	0, 1, 2, 2, 3, 0
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
};

struct PushUniforms {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 mvp;
};

class vkRenderObject
{
public:
	vkRenderObject(uint32_t imageCount, std::vector<Vertex> vertices, std::vector<uint32_t> indices,
		vk::DescriptorSetLayout &descriptorSetLayout, vk::DescriptorPool &descriptorPool, 
		vkTexture* texture, vkCommandBufferManager* commandBufferManager, vkTexture* debugTexture);
	virtual ~vkRenderObject();

	std::vector<Vertex> get_vertices() { return mVertices; }
	std::vector<uint32_t> get_indices() { return mIndices; }

	vk::Buffer get_vertex_buffer() { return mVertexBuffer.get_vkk_buffer(); }
	//void setVertexBuffer(vk::Buffer vertexBuffer) { _vertexBuffer = vertexBuffer; }
	vk::Buffer get_index_buffer() { return mIndexBuffer.get_vkk_buffer(); }
	//void setIndexBuffer(vk::Buffer indexBuffer) { _indexBuffer = indexBuffer; }

	std::vector<vk::DescriptorSet> get_descriptor_sets() { return mDescriptorSets; }
	vk::DescriptorSet& get_descriptor_set() { return mDescriptorSets[vkContext::instance().currentFrame]; }
	PushUniforms get_push_uniforms() { return mPushUniforms; }

	void update_uniform_buffer(uint32_t currentImage, UniformBufferObject ubo);

private:
	uint32_t mImageCount;

	std::vector<Vertex> mVertices;
	std::vector<uint32_t> mIndices;

	vkCgbBuffer mVertexBuffer;
	vkCgbBuffer mIndexBuffer;

	std::vector<vkCgbBuffer*> mUniformBuffers;

	PushUniforms mPushUniforms;
	// TODO PERFORMANCE use multiple descriptor sets / per render pass, e.g. shadow pass does not use textures
	// or like suggested for AMD Hardware, use one large descriptor set for everything and index into Texture arrays and uniforms
	// suggestion: use an array of descriptor sets, the drawer then can decide, which descriptor set to use, 
	// additionally allow global/per drawer descriptor sets
	// this offers the most flexibility for the user of the framework, while still being easy to use
	std::vector<vk::DescriptorSet> mDescriptorSets;

	void create_uniform_buffer(vkCommandBufferManager* commandBufferManager);
	void create_descriptor_sets(vk::DescriptorSetLayout &descriptorSetLayout, vk::DescriptorPool &descriptorPool, 
		vkTexture* texture, vkTexture* debugTexture);
};

