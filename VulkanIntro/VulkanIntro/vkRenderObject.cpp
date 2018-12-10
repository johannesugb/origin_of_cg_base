#include "vkRenderObject.h"



vkRenderObject::vkRenderObject(uint32_t imageCount, std::vector<Vertex> vertices, std::vector<uint32_t> indices,
	VkDescriptorSetLayout &descriptorSetLayout, VkDescriptorPool &descriptorPool, 
	vkTexture* texture, vkCommandBufferManager* commandBufferManager)
	: _imageCount(imageCount), _vertices(vertices), _indices(indices),
	_vertexBuffer(sizeof(_vertices[0]) * _vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, commandBufferManager, _vertices.data()),
	_indexBuffer(sizeof(_indices[0]) * _indices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, commandBufferManager, _indices.data())
{
	createUniformBuffer(commandBufferManager);
	createDescriptorSets(descriptorSetLayout, descriptorPool, texture);
}


vkRenderObject::~vkRenderObject()
{
	for (size_t i = 0; i < _imageCount; i++) {
		delete _uniformBuffers[i];
	}
}

void vkRenderObject::createUniformBuffer(vkCommandBufferManager* commandBufferManager) {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	_uniformBuffers.resize(_imageCount);

	for (size_t i = 0; i < _imageCount; i++) {
		_uniformBuffers[i] = new vkCgbBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, commandBufferManager);
	}
}

void vkRenderObject::createDescriptorSets(VkDescriptorSetLayout &descriptorSetLayout, VkDescriptorPool &descriptorPool, vkTexture* texture) {
	std::vector<VkDescriptorSetLayout> layouts(_imageCount, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(_imageCount);
	allocInfo.pSetLayouts = layouts.data();

	_descriptorSets.resize(_imageCount);
	if (vkAllocateDescriptorSets(vkContext::instance().device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < _imageCount; i++) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = _uniformBuffers[i]->getVkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture->getTextureImageView();
		imageInfo.sampler = texture->getTextureSampler();

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = _descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = _descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(vkContext::instance().device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void vkRenderObject::updateUniformBuffer(uint32_t currentImage, float time, VkExtent2D swapChainExtent) {
	// TODO transfer code to camera
	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;
	ubo.mvp = ubo.proj * ubo.view * ubo.model;

	_pushUniforms.model = ubo.model;
	_pushUniforms.view = ubo.view;
	_pushUniforms.proj = ubo.proj;
	_pushUniforms.mvp = ubo.mvp;

	_uniformBuffers[currentImage]->updateBuffer(&ubo, sizeof(ubo));
}