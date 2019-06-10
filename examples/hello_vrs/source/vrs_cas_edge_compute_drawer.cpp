#include "vrs_cas_edge_compute_drawer.h"



vrs_cas_edge_compute_drawer::vrs_cas_edge_compute_drawer(std::shared_ptr<cgb::vulkan_command_buffer_manager> commandBufferManager, std::shared_ptr<cgb::vulkan_pipeline> pipeline,
	std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> globalResourceBundles, std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages,
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsPrevRenderBlitImages, std::vector<std::shared_ptr<cgb::vulkan_image>> vrsEdgeImages,
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsEdgeBlitImages) : vrs_cas_compute_drawer(commandBufferManager, pipeline, globalResourceBundles,
		mVrsPrevRenderImages, vrsPrevRenderBlitImages),
	mVrsEdgeImages(vrsEdgeImages), mVrsEdgeBlitImages(vrsEdgeBlitImages)
{
}


vrs_cas_edge_compute_drawer::~vrs_cas_edge_compute_drawer()
{
}

void vrs_cas_edge_compute_drawer::blit_image(vk::CommandBuffer& commandBuffer)
{
	// blit parent images
	vrs_cas_compute_drawer::blit_image(commandBuffer);

	auto currentIdx = cgb::vulkan_context::instance().currentFrame;
	int32_t blitWidth = mVrsEdgeBlitImages[currentIdx]->get_width();
	int32_t blitHeight = mVrsEdgeBlitImages[currentIdx]->get_height();
	int32_t width = mVrsEdgeImages[currentIdx]->get_width();
	int32_t height = mVrsEdgeImages[currentIdx]->get_height();

	vk::ImageMemoryBarrier barrier = {};
	barrier.image = mVrsEdgeBlitImages[currentIdx]->get_image();
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

	vk::ImageMemoryBarrier srcBarrier = barrier;
	srcBarrier.image = mVrsEdgeImages[currentIdx]->get_image();
	srcBarrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	srcBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
	srcBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
	srcBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eTransfer, {},
		{}, {}, { barrier, srcBarrier });

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
		mVrsEdgeImages[currentIdx]->get_image(), vk::ImageLayout::eTransferSrcOptimal,
		mVrsEdgeBlitImages[currentIdx]->get_image(), vk::ImageLayout::eTransferDstOptimal,
		1, &blit,
		vk::Filter::eLinear);

	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eGeneral;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	srcBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
	srcBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	srcBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	srcBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, {},
		{}, {}, { barrier, srcBarrier });
}