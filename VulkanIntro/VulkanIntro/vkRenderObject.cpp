#include "vkRenderObject.h"



vkRenderObject::vkRenderObject(uint32_t imageCount, std::vector<Vertex> vertices, std::vector<uint32_t> indices,
	vk::DescriptorSetLayout &descriptorSetLayout, vk::DescriptorPool &descriptorPool, 
	vkTexture* texture, vkCommandBufferManager* commandBufferManager, vkTexture* debugTexture)
	: mImageCount(imageCount), mVertices(vertices), mIndices(indices),
	mVertexBuffer(sizeof(mVertices[0]) * mVertices.size(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, commandBufferManager, mVertices.data()),
	mIndexBuffer(sizeof(mIndices[0]) * mIndices.size(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, commandBufferManager, mIndices.data())
{
	create_uniform_buffer(commandBufferManager);
	create_descriptor_sets(descriptorSetLayout, descriptorPool, texture, debugTexture);
}


vkRenderObject::~vkRenderObject()
{
	for (size_t i = 0; i < mImageCount; i++) {
		delete mUniformBuffers[i];
	}
}

void vkRenderObject::create_uniform_buffer(vkCommandBufferManager* commandBufferManager) {
	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	mUniformBuffers.resize(mImageCount);

	for (size_t i = 0; i < mImageCount; i++) {
		mUniformBuffers[i] = new vkCgbBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, commandBufferManager);
	}
}

void vkRenderObject::create_descriptor_sets(vk::DescriptorSetLayout &descriptorSetLayout, vk::DescriptorPool &descriptorPool, vkTexture* texture, vkTexture* debugTexture) {
	std::vector<vk::DescriptorSetLayout> layouts(mImageCount, descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo = {};
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(mImageCount);
	allocInfo.pSetLayouts = layouts.data();

	mDescriptorSets.resize(mImageCount);
	if (vkContext::instance().device.allocateDescriptorSets(&allocInfo, mDescriptorSets.data()) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < mImageCount; i++) {
		vk::DescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = mUniformBuffers[i]->get_vkk_buffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		vk::DescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageInfo.imageView = texture->getTextureImageView();
		imageInfo.sampler = texture->getTextureSampler();

		vk::DescriptorImageInfo imageDebugInfo = {};
		imageDebugInfo.imageLayout = vk::ImageLayout::eGeneral;
		imageDebugInfo.imageView = debugTexture->getTextureImageView();
		imageDebugInfo.sampler = debugTexture->getTextureSampler();

		std::array<vk::WriteDescriptorSet, 3> descriptorWrites = {};

		descriptorWrites[0].dstSet = mDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].dstSet = mDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		descriptorWrites[2].dstSet = mDescriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &imageDebugInfo;

		vkContext::instance().device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void vkRenderObject::update_uniform_buffer(uint32_t currentImage, float time, vk::Extent2D swapChainExtent) {
	// TODO transfer code to camera
	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;
	ubo.mvp = ubo.proj * ubo.view * ubo.model;

	mPushUniforms.model = ubo.model;
	mPushUniforms.view = ubo.view;
	mPushUniforms.proj = ubo.proj;
	mPushUniforms.mvp = ubo.mvp;

	mUniformBuffers[currentImage]->update_buffer(&ubo, sizeof(ubo));
}