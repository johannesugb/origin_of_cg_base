#include "vrs_cas_compute_drawer.h"



vrs_cas_compute_drawer::vrs_cas_compute_drawer(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline,
	std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> globalResourceBundles, std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages,
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsPrevRenderBlitImages) : vulkan_drawer(commandBufferManager, pipeline, globalResourceBundles),
	mVrsPrevRenderImages(mVrsPrevRenderImages), mVrsPrevRenderBlitImages(vrsPrevRenderBlitImages)
{
}


vrs_cas_compute_drawer::~vrs_cas_compute_drawer()
{
}


void vrs_cas_compute_drawer::draw(std::vector<cgb::vulkan_render_object*> renderObjects)
{
	vk::CommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	vk::CommandBuffer commandBuffer = mCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::eSecondary, beginInfo);

	// blit image
	blit_image(commandBuffer);

	// bind pipeline for this compute command
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline->get_pipeline());

	vk::ImageMemoryBarrier imgMemBarrier = {};
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

	auto globalDescriptorSets = get_descriptor_sets(mGlobalResourceBundles);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mPipeline->get_pipeline_layout(), 0, globalDescriptorSets.size(), globalDescriptorSets.data(), 0, nullptr);

	commandBuffer.dispatch(std::ceil(mWidth * 1.0 / WORKGROUP_SIZE), std::ceil(mHeight * 1.0 / WORKGROUP_SIZE), 1);

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

void vrs_cas_compute_drawer::blit_image(vk::CommandBuffer& commandBuffer)
{
	auto currentIdx = cgb::vulkan_context::instance().currentFrame;
	int32_t blitWidth = mVrsPrevRenderBlitImages[currentIdx]->get_width();
	int32_t blitHeight = mVrsPrevRenderBlitImages[currentIdx]->get_height();
	int32_t width = mVrsPrevRenderImages[currentIdx]->get_width();
	int32_t height = mVrsPrevRenderImages[currentIdx]->get_height();

	vk::ImageMemoryBarrier barrier = {};
	barrier.image = mVrsPrevRenderBlitImages[currentIdx]->get_image();
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseMipLevel = 0;

	barrier.oldLayout = vk::ImageLayout::eUndefined;
	barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
	barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eTransfer, {},
		0, nullptr,
		0, nullptr,
		1, &barrier);

	vk::ImageBlit blit = {};
	blit.srcOffsets[0] = { 0, 0, 0 };
	blit.srcOffsets[1] = { width, height, 1 };
	blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;
	blit.dstOffsets[0] = { 0, 0, 0 };
	blit.dstOffsets[1] = { blitWidth, blitHeight, 1 };
	blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;

	commandBuffer.blitImage(
		mVrsPrevRenderImages[currentIdx]->get_image(), vk::ImageLayout::eTransferSrcOptimal,
		mVrsPrevRenderBlitImages[currentIdx]->get_image(), vk::ImageLayout::eTransferDstOptimal,
		1, &blit,
		vk::Filter::eLinear);

	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eGeneral;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {},
		0, nullptr,
		0, nullptr,
		1, &barrier);
}