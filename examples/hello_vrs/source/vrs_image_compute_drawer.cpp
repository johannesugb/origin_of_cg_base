#include "vrs_image_compute_drawer.h"

namespace cgb {

	vrs_image_compute_drawer::vrs_image_compute_drawer(std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<vulkan_pipeline> pipeline,
		std::vector<std::shared_ptr<vulkan_image>> vrsDebugImages) : vulkan_drawer(commandBufferManager, pipeline), mVrsDebugImages(vrsDebugImages)
	{
		createVrsComputeDescriptorSetLayout();
		createVrsComputeDescriptorPool();
		createVrsDescriptorSets();

		pipeline->add_resource_bundle_layout(std::make_shared<cgb::vulkan_resource_bundle_layout>(vrsComputeDebugDescriptorSetLayout));
		pipeline->bake();
	}


	vrs_image_compute_drawer::~vrs_image_compute_drawer()
	{
		vulkan_context::instance().device.destroyDescriptorPool(vrsComputeDebugDescriptorPool);
	}

	void vrs_image_compute_drawer::draw(std::vector<vulkan_render_object*> renderObjects, std::shared_ptr<cgb::vulkan_framebuffer> framebuffer)
	{
		vk::CommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;

		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
		beginInfo.pInheritanceInfo = &inheritanceInfo;

		vk::CommandBuffer commandBuffer = mCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::eSecondary, beginInfo);

		// bind pipeline for this compute command
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline->get_pipeline());

		vk::ImageMemoryBarrier imgMemBarrier = {};
		imgMemBarrier.srcAccessMask = {};
		imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
		imgMemBarrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imgMemBarrier.newLayout = vk::ImageLayout::eGeneral;
		imgMemBarrier.image = mVrsDebugImages[vulkan_context::instance().currentFrame]->get_image();
		imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imgMemBarrier.subresourceRange.baseArrayLayer = 0;
		imgMemBarrier.subresourceRange.layerCount = 1;
		imgMemBarrier.subresourceRange.levelCount = 1;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eComputeShader, {}, nullptr, nullptr, imgMemBarrier);

		imgMemBarrier = {};
		imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eShadingRateImageReadNV;
		imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
		imgMemBarrier.oldLayout = vk::ImageLayout::eShadingRateOptimalNV;
		imgMemBarrier.newLayout = vk::ImageLayout::eGeneral;
		imgMemBarrier.image = mVrsImages[vulkan_context::instance().currentFrame]->get_image();
		imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imgMemBarrier.subresourceRange.baseArrayLayer = 0;
		imgMemBarrier.subresourceRange.layerCount = 1;
		imgMemBarrier.subresourceRange.levelCount = 1;
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eShadingRateImageNV, vk::PipelineStageFlagBits::eComputeShader, {}, nullptr, nullptr, imgMemBarrier);


		std::vector<vk::DescriptorSet> descriptorSetsToBind = { mDescriptorSets[vulkan_context::instance().currentFrame], mVrsComputeDebugDescriptorSets[vulkan_context::instance().currentFrame] };
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mPipeline->get_pipeline_layout(), 0, 2, descriptorSetsToBind.data(), 0, nullptr);

		auto eyeData = mEyeInf->get_eyetracking_data();
		auto data = vrs_eye_comp_data();
		data.eyePoint = glm::vec2(eyeData.positionX, eyeData.positionY);
		data.imgSize = glm::vec2(mWidth, mHeight);

		auto arr = vk::ArrayProxy<const vrs_eye_comp_data>(data);
		commandBuffer.pushConstants(mPipeline->get_pipeline_layout(), vk::ShaderStageFlagBits::eCompute, 0, arr);

		commandBuffer.dispatch(std::ceil(mWidth * 1.0 / WORKGROUP_SIZE), std::ceil(mHeight * 1.0 / WORKGROUP_SIZE), 1);

		imgMemBarrier = {};
		imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
		imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		imgMemBarrier.oldLayout = vk::ImageLayout::eGeneral;
		imgMemBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imgMemBarrier.image = mVrsDebugImages[vulkan_context::instance().currentFrame]->get_image();
		imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imgMemBarrier.subresourceRange.baseArrayLayer = 0;
		imgMemBarrier.subresourceRange.layerCount = 1;
		imgMemBarrier.subresourceRange.levelCount = 1;
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, imgMemBarrier);

		imgMemBarrier = {};
		imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
		imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eShadingRateImageReadNV;
		imgMemBarrier.oldLayout = vk::ImageLayout::eGeneral;
		imgMemBarrier.newLayout = vk::ImageLayout::eShadingRateOptimalNV;
		imgMemBarrier.image = mVrsImages[vulkan_context::instance().currentFrame]->get_image();
		imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imgMemBarrier.subresourceRange.baseArrayLayer = 0;
		imgMemBarrier.subresourceRange.layerCount = 1;
		imgMemBarrier.subresourceRange.levelCount = 1;
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eShadingRateImageNV, {}, nullptr, nullptr, imgMemBarrier);
	}

	void vrs_image_compute_drawer::createVrsComputeDescriptorSetLayout() {
		vk::DescriptorSetLayoutBinding storageImageLayoutBinding = {};
		storageImageLayoutBinding.binding = 0;
		storageImageLayoutBinding.descriptorCount = 1;
		storageImageLayoutBinding.descriptorType = vk::DescriptorType::eStorageImage;
		storageImageLayoutBinding.pImmutableSamplers = nullptr;
		storageImageLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eCompute;

		vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
		std::array<vk::DescriptorSetLayoutBinding, 1> bindings = { storageImageLayoutBinding };
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();


		if (vulkan_context::instance().device.createDescriptorSetLayout(&layoutInfo, nullptr, &vrsComputeDebugDescriptorSetLayout) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create vrs compute descriptor set layout!");
		}
	}

	void vrs_image_compute_drawer::createVrsComputeDescriptorPool() {
		std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
		poolSizes[0].type = vk::DescriptorType::eStorageImage;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(vulkan_context::instance().dynamicRessourceCount);

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(vulkan_context::instance().dynamicRessourceCount);

		if (vulkan_context::instance().device.createDescriptorPool(&poolInfo, nullptr, &vrsComputeDebugDescriptorPool) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create vrs compute descriptor pool!");
		}
	}

	void vrs_image_compute_drawer::createVrsDescriptorSets() {
		std::vector<vk::DescriptorSetLayout> layouts(vulkan_context::instance().dynamicRessourceCount, vrsComputeDebugDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.descriptorPool = vrsComputeDebugDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan_context::instance().dynamicRessourceCount);
		allocInfo.pSetLayouts = layouts.data();

		mVrsComputeDebugDescriptorSets.resize(vulkan_context::instance().dynamicRessourceCount);
		if (vulkan_context::instance().device.allocateDescriptorSets(&allocInfo, mVrsComputeDebugDescriptorSets.data()) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < vulkan_context::instance().dynamicRessourceCount; i++) {
			vk::DescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = vk::ImageLayout::eGeneral;
			imageInfo.imageView = mVrsDebugImages[i]->get_image_view();

			std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {};

			descriptorWrites[0].dstSet = mVrsComputeDebugDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = vk::DescriptorType::eStorageImage;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;

			vulkan_context::instance().device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

}