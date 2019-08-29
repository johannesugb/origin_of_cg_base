#include "vrs_eye_tracked_blit.h"

struct vrs_eye_comp_data {
	glm::vec2 eyePoint;
	glm::vec2 imgSize;
};


vrs_eye_tracked_blit::vrs_eye_tracked_blit(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline,
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsDebugImages) : vrs_image_compute_drawer_base(commandBufferManager, pipeline, {}, vrsDebugImages)
{
	createVrsComputeDescriptorSetLayout();
	createVrsComputeDescriptorPool();
	createVrsDescriptorSets();

	pipeline->add_resource_bundle_layout(std::make_shared<cgb::vulkan_resource_bundle_layout>(vrsComputeDebugDescriptorSetLayout));
	pipeline->bake();
}


vrs_eye_tracked_blit::~vrs_eye_tracked_blit()
{
	cgb::vulkan_context::instance().device.destroyDescriptorPool(vrsComputeDebugDescriptorPool);
}

void vrs_eye_tracked_blit::draw(std::vector<cgb::vulkan_render_object*> renderObjects, std::shared_ptr<cgb::vulkan_framebuffer> framebuffer)
{
	vk::CommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	vk::CommandBuffer commandBuffer = mCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::eSecondary, beginInfo);

	vk::ImageMemoryBarrier imgMemBarrier = {};
	imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eShadingRateImageReadNV;
	imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
	imgMemBarrier.oldLayout = vk::ImageLayout::eShadingRateOptimalNV;
	imgMemBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
	imgMemBarrier.image = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_image();
	imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imgMemBarrier.subresourceRange.baseArrayLayer = 0;
	imgMemBarrier.subresourceRange.layerCount = 1;
	imgMemBarrier.subresourceRange.levelCount = 1;
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eShadingRateImageNV, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imgMemBarrier);

	auto eyeData = mEyeInf->get_eyetracking_data();
	auto eyePos = glm::vec2(1) - glm::vec2(eyeData.positionX, eyeData.positionY);

	int32_t width = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_width();
	int32_t height = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_height();
	int32_t offsetX = std::min(std::max(0, int32_t(eyePos.x * width)), width);
	int32_t offsetY = std::min(std::max(0, int32_t(eyePos.y * height)), height);

	vk::ImageBlit blit = {};
	blit.srcOffsets[0] = { offsetX , offsetY, 0 };
	blit.srcOffsets[1] = { width + offsetX, height + offsetY, 1 };
	blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;
	blit.dstOffsets[0] = { 0, 0, 0 };
	blit.dstOffsets[1] = { width, height, 1 };
	blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;

	commandBuffer.blitImage(
		mPrecomputedImage->get_image(), vk::ImageLayout::eTransferSrcOptimal,
		mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_image(), vk::ImageLayout::eTransferDstOptimal,
		1, &blit,
		vk::Filter::eNearest);


	imgMemBarrier = {};
	imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eShadingRateImageReadNV;
	imgMemBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	imgMemBarrier.newLayout = vk::ImageLayout::eShadingRateOptimalNV;
	imgMemBarrier.image = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_image();
	imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imgMemBarrier.subresourceRange.baseArrayLayer = 0;
	imgMemBarrier.subresourceRange.layerCount = 1;
	imgMemBarrier.subresourceRange.levelCount = 1;
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eShadingRateImageNV, {}, nullptr, nullptr, imgMemBarrier);

}

void vrs_eye_tracked_blit::precompute()
{

	auto width = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_width();
	auto height = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_height();
	auto mipLevels = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_mip_levels();
	auto numSamples = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_num_samples();
	auto format = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_format();
	auto tiling = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_tiling();
	mPrecomputedImage = std::make_unique<cgb::vulkan_image>(width * 2, height * 2, mipLevels, 1, numSamples, format, tiling,
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eShadingRateImageNV, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
		

	// draw in initial vrs image
	vk::CommandBuffer commandBuffer = mCommandBufferManager->begin_single_time_commands();

	// draw default image
	draw_initial(commandBuffer);

	// blit default image into big image
	blit_initial_image(commandBuffer);


	mCommandBufferManager->end_single_time_commands(commandBuffer);
}

void vrs_eye_tracked_blit::createVrsComputeDescriptorSetLayout() {
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


	if (cgb::vulkan_context::instance().device.createDescriptorSetLayout(&layoutInfo, nullptr, &vrsComputeDebugDescriptorSetLayout) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create vrs compute descriptor set layout!");
	}
}

void vrs_eye_tracked_blit::createVrsComputeDescriptorPool() {
	std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
	poolSizes[0].type = vk::DescriptorType::eStorageImage;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(cgb::vulkan_context::instance().dynamicRessourceCount);

	vk::DescriptorPoolCreateInfo poolInfo = {};
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(cgb::vulkan_context::instance().dynamicRessourceCount);

	if (cgb::vulkan_context::instance().device.createDescriptorPool(&poolInfo, nullptr, &vrsComputeDebugDescriptorPool) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create vrs compute descriptor pool!");
	}
}

void vrs_eye_tracked_blit::createVrsDescriptorSets() {
	std::vector<vk::DescriptorSetLayout> layouts(cgb::vulkan_context::instance().dynamicRessourceCount, vrsComputeDebugDescriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo = {};
	allocInfo.descriptorPool = vrsComputeDebugDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(cgb::vulkan_context::instance().dynamicRessourceCount);
	allocInfo.pSetLayouts = layouts.data();

	mVrsComputeDebugDescriptorSets.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
	if (cgb::vulkan_context::instance().device.allocateDescriptorSets(&allocInfo, mVrsComputeDebugDescriptorSets.data()) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
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

		cgb::vulkan_context::instance().device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void vrs_eye_tracked_blit::draw_initial(vk::CommandBuffer& commandBuffer)
{
	// bind pipeline for this compute command
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline->get_pipeline());

	vk::ImageMemoryBarrier imgMemBarrier = {};
	imgMemBarrier.srcAccessMask = {};
	imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
	imgMemBarrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imgMemBarrier.newLayout = vk::ImageLayout::eGeneral;
	imgMemBarrier.image = mVrsDebugImages[cgb::vulkan_context::instance().currentFrame]->get_image();
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
	imgMemBarrier.image = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_image();
	imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imgMemBarrier.subresourceRange.baseArrayLayer = 0;
	imgMemBarrier.subresourceRange.layerCount = 1;
	imgMemBarrier.subresourceRange.levelCount = 1;
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eShadingRateImageNV, vk::PipelineStageFlagBits::eComputeShader, {}, nullptr, nullptr, imgMemBarrier);


	std::vector<vk::DescriptorSet> descriptorSetsToBind = { mDescriptorSets[cgb::vulkan_context::instance().currentFrame], mVrsComputeDebugDescriptorSets[cgb::vulkan_context::instance().currentFrame] };
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mPipeline->get_pipeline_layout(), 0, 2, descriptorSetsToBind.data(), 0, nullptr);

	auto data = vrs_eye_comp_data();
	data.eyePoint = glm::vec2(0.5, 0.5);
	data.imgSize = glm::vec2(mWidth, mHeight);

	auto arr = vk::ArrayProxy<const vrs_eye_comp_data>(data);
	commandBuffer.pushConstants(mPipeline->get_pipeline_layout(), vk::ShaderStageFlagBits::eCompute, 0, arr);

	commandBuffer.dispatch(std::ceil(mWidth * 1.0 / WORKGROUP_SIZE), std::ceil(mHeight * 1.0 / WORKGROUP_SIZE), 1);

	imgMemBarrier = {};
	imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
	imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	imgMemBarrier.oldLayout = vk::ImageLayout::eGeneral;
	imgMemBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imgMemBarrier.image = mVrsDebugImages[cgb::vulkan_context::instance().currentFrame]->get_image();
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
	imgMemBarrier.image = mVrsImages[cgb::vulkan_context::instance().currentFrame]->get_image();
	imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imgMemBarrier.subresourceRange.baseArrayLayer = 0;
	imgMemBarrier.subresourceRange.layerCount = 1;
	imgMemBarrier.subresourceRange.levelCount = 1;
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eShadingRateImageNV, {}, nullptr, nullptr, imgMemBarrier);

}

void vrs_eye_tracked_blit::blit_initial_image(vk::CommandBuffer& commandBuffer)
{
	// clear big image
	vk::ImageMemoryBarrier imgMemBarrier = {};
	imgMemBarrier.srcAccessMask = {};
	imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
	imgMemBarrier.oldLayout = vk::ImageLayout::eUndefined;
	imgMemBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
	imgMemBarrier.image = mPrecomputedImage->get_image();
	imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imgMemBarrier.subresourceRange.baseArrayLayer = 0;
	imgMemBarrier.subresourceRange.layerCount = 1;
	imgMemBarrier.subresourceRange.levelCount = 1;
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imgMemBarrier);
		
	auto defaultShadingRate = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f }));
	commandBuffer.clearColorImage(mPrecomputedImage->get_image(), vk::ImageLayout::eTransferDstOptimal, defaultShadingRate, imgMemBarrier.subresourceRange);

	// blit computed default image into center
	auto currentIdx = cgb::vulkan_context::instance().currentFrame;
	int32_t width = mVrsImages[currentIdx]->get_width();
	int32_t height = mVrsImages[currentIdx]->get_height();

	imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eShadingRateImageReadNV;
	imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
	imgMemBarrier.oldLayout = vk::ImageLayout::eShadingRateOptimalNV;
	imgMemBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
	imgMemBarrier.image = mVrsImages[currentIdx]->get_image();
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eShadingRateImageNV, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imgMemBarrier);

	vk::ImageBlit blit = {};
	blit.srcOffsets[0] = { 0, 0, 0 };
	blit.srcOffsets[1] = { width, height, 1 };
	blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;
	blit.dstOffsets[0] = { width/2, height/2, 0 };
	blit.dstOffsets[1] = { width + width/2, height+height/2, 1 };
	blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;

	commandBuffer.blitImage(
		mVrsImages[currentIdx]->get_image(), vk::ImageLayout::eTransferSrcOptimal,
		mPrecomputedImage->get_image(), vk::ImageLayout::eTransferDstOptimal,
		1, &blit,
		vk::Filter::eNearest);

	imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eShadingRateImageReadNV;
	imgMemBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
	imgMemBarrier.newLayout = vk::ImageLayout::eShadingRateOptimalNV;
	imgMemBarrier.image = mVrsImages[currentIdx]->get_image();

	auto precomputedBarrier = imgMemBarrier;
	imgMemBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	imgMemBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
	imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
	imgMemBarrier.image = mPrecomputedImage->get_image();

	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eShadingRateImageNV, {},
		{}, {}, { imgMemBarrier, precomputedBarrier });
}