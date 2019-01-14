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

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 1;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static VkVertexInputBindingDescription getBindingDescription2() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 2;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		attributeDescriptions[0].binding = 1;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 2;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 2;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
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
		VkDescriptorSetLayout &descriptorSetLayout, VkDescriptorPool &descriptorPool, 
		vkTexture* texture, vkCommandBufferManager* commandBufferManager);
	virtual ~vkRenderObject();

	std::vector<Vertex> get_vertices() { return _vertices; }
	std::vector<uint32_t> get_indices() { return _indices; }

	VkBuffer getVertexBuffer() { return _vertexBuffer.getVkBuffer(); }
	//void setVertexBuffer(VkBuffer vertexBuffer) { _vertexBuffer = vertexBuffer; }
	VkBuffer getIndexBuffer() { return _indexBuffer.getVkBuffer(); }
	//void setIndexBuffer(VkBuffer indexBuffer) { _indexBuffer = indexBuffer; }

	std::vector<VkDescriptorSet> getDescriptorSets() { return _descriptorSets; }
	VkDescriptorSet& getDescriptorSet() { return _descriptorSets[vkContext::instance().currentFrame]; }
	PushUniforms getPushUniforms() { return _pushUniforms; }

	void updateUniformBuffer(uint32_t currentImage, float time, VkExtent2D swapChainExtent);

private:
	uint32_t _imageCount;

	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;

	vkCgbBuffer _vertexBuffer;
	vkCgbBuffer _indexBuffer;

	std::vector<vkCgbBuffer*> _uniformBuffers;

	PushUniforms _pushUniforms;
	// TODO PERFORMANCE use multiple descriptor sets / per render pass, e.g. shadow pass does not use textures
	// or like suggested for AMD Hardware, use one large descriptor set for everything and index into Texture arrays and uniforms
	// suggestion: use an array of descriptor sets, the drawer then can decide, which descriptor set to use, 
	// additionally allow global/per drawer descriptor sets
	// this offers the most flexibility for the user of the framework, while still being easy to use
	std::vector<VkDescriptorSet> _descriptorSets;

	void createUniformBuffer(vkCommandBufferManager* commandBufferManager);
	void createDescriptorSets(VkDescriptorSetLayout &descriptorSetLayout, VkDescriptorPool &descriptorPool, 
		vkTexture* texture);
};

