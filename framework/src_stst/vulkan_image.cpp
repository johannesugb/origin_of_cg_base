#include "vulkan_image.h"

#include <algorithm>
#include <stdexcept>

#include "vulkan_buffer.h"
#include "vulkan_memory_manager.h"

namespace cgb {

	cgb::vulkan_image::vulkan_image(vk::Image image, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t texChannels,
		vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspects, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager) :
		mImage(image), mTexWidth(width), mTexHeight(height), mMipLevels(mipLevels), mTtexChannels(texChannels),
		mNumSamples(numSamples), mFormat(format), mUsage(usage), mAspects(aspects), mCommandBufferManager(commandBufferManager)
	{
		mImageView = create_image_view(mFormat, mAspects, mMipLevels);
		vkImageNotControlled = true;

		mTiling = {};
	}

	cgb::vulkan_image::vulkan_image(vk::Image image, vk::ImageView imageView, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t texChannels, 
		vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspects, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager) :
		mImage(image), mImageView(imageView), mTexWidth(width), mTexHeight(height), mMipLevels(mipLevels), mTtexChannels(texChannels),
		mNumSamples(numSamples), mFormat(format), mUsage(usage), mAspects(aspects), mCommandBufferManager(commandBufferManager)
	{
		vkImageViewNotControlled = true;
		vkImageNotControlled = true;

		mTiling = {};
	}

	vulkan_image::vulkan_image(void* pixels, uint32_t texWidth, uint32_t texHeight, int texChannels, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager) :
		mCommandBufferManager(commandBufferManager), mTexWidth(texWidth), mTexHeight(texHeight), mTtexChannels(texChannels)
	{
		create_texture_image(pixels, texWidth, texHeight, texChannels);
		create_texture_image_view();
	}

	vulkan_image::vulkan_image(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t texChannels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspects, std::shared_ptr<vulkan_command_buffer_manager> commandBufferManager) : 
		mCommandBufferManager(commandBufferManager), mTexWidth(width), mTexHeight(height), mMipLevels(mipLevels), mTtexChannels(texChannels), 
		mNumSamples(numSamples), mFormat(format), mTiling(tiling), mUsage(usage), mMemoryProperties(properties), mAspects(aspects)
	{
		create_image(mTexWidth, mTexHeight, mMipLevels, numSamples, format, tiling, usage, properties, mImage, mImageMemory);
		mImageView = create_image_view(format, aspects, mipLevels);
	}


	vulkan_image::~vulkan_image()
	{
		if (!vkImageViewNotControlled) {
			vkDestroyImageView(vulkan_context::instance().vulkan_context::instance().device, mImageView, nullptr);
		}
		if (!vkImageNotControlled) {
			vkDestroyImage(vulkan_context::instance().vulkan_context::instance().device, mImage, nullptr);
			vulkan_context::instance().memoryManager->free_memory(mImageMemory);
		}
	}

	void vulkan_image::create_texture_image(void* pixels, uint32_t texWidth, uint32_t texHeight, int texChannels) {
		mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		

		mFormat = vk::Format::eR8G8B8A8Unorm;
		mTtexChannels = 4; // currently default to this due to format
		/*switch (texChannels) {
		case 3:
			format = vk::Format::eR8G8B8Unorm;
			break;
		case 2:
			format = vk::Format::eR8G8Unorm;
			break;
		case 1:
			format = vk::Format::eR8Unorm;
			break;
		}*/

		create_image(texWidth, texHeight, mMipLevels, vk::SampleCountFlagBits::e1, mFormat, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, mImage, mImageMemory);

		update_pixels(pixels);

		if (mFormat == vk::Format::eR8G8B8A8Unorm) {
			generate_mipmaps(mFormat, texWidth, texHeight, mMipLevels);
		}
	}

	void vulkan_image::create_image(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties, vk::Image& image, vulkan_memory& imageMemory) {
		vk::ImageCreateInfo imageInfo = {};
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;

		if (vulkan_context::instance().device.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create image!");
		}

		vk::MemoryRequirements memRequirements;
		vulkan_context::instance().device.getImageMemoryRequirements(image, &memRequirements);

		vulkan_context::instance().memoryManager->allocate_memory(memRequirements, properties, imageMemory);

		vkBindImageMemory(vulkan_context::instance().device, image, imageMemory.memory, imageMemory.offset);
	}

	void vulkan_image::transition_image_layout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels) {
		vk::CommandBuffer commandBuffer = mCommandBufferManager->begin_single_time_commands();

		vk::ImageMemoryBarrier barrier = {};
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = mImage;

		if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

			if (has_stencil_component(format)) {
				barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferSrcOptimal) {
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eGeneral) {
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShadingRateImageReadNV;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eShadingRateImageNV;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eShadingRateOptimalNV) {
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eShadingRateImageReadNV;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eShadingRateImageNV;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		commandBuffer.pipelineBarrier(
			sourceStage, destinationStage,
			{},
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		mCommandBufferManager->end_single_time_commands(commandBuffer);
	}

	void vulkan_image::update_pixels(void* pixels) {
		vk::DeviceSize imageSize = vk::DeviceSize(mTexWidth) * vk::DeviceSize(mTexHeight) * vk::DeviceSize(mTtexChannels);
		vulkan_buffer stagingBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, mCommandBufferManager);
		stagingBuffer.update_buffer(pixels, imageSize);

		transition_image_layout(mFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mMipLevels);
		copy_buffer_to_image(stagingBuffer, mImage, static_cast<uint32_t>(mTexWidth), static_cast<uint32_t>(mTexHeight));
	}

	bool vulkan_image::has_stencil_component(vk::Format format) {
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	void vulkan_image::copy_buffer_to_image(vulkan_buffer& buffer, vk::Image image, uint32_t width, uint32_t height) {
		vk::CommandBuffer commandBuffer = mCommandBufferManager->begin_single_time_commands();

		vk::BufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		commandBuffer.copyBufferToImage(
			buffer.get_vk_buffer(),
			image,
			vk::ImageLayout::eTransferDstOptimal,
			1,
			&region
		);

		mCommandBufferManager->end_single_time_commands(commandBuffer);
	}

	void vulkan_image::generate_mipmaps(vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
		// Check if image format supports linear blitting
		vk::FormatProperties formatProperties;
		vulkan_context::instance().physicalDevice.getFormatProperties(imageFormat, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		vk::CommandBuffer commandBuffer = mCommandBufferManager->begin_single_time_commands();

		vk::ImageMemoryBarrier barrier = {};
		barrier.image = mImage;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {},
				0, nullptr,
				0, nullptr,
				1, &barrier);

			vk::ImageBlit blit = {};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			commandBuffer.blitImage(
				mImage, vk::ImageLayout::eTransferSrcOptimal,
				mImage, vk::ImageLayout::eTransferDstOptimal,
				1, &blit,
				vk::Filter::eLinear);

			barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
			0, nullptr,
			0, nullptr,
			1, &barrier);

		mCommandBufferManager->end_single_time_commands(commandBuffer);
	}

	void vulkan_image::create_texture_image_view() {
		mImageView = create_image_view(vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, mMipLevels);
	}

	vk::ImageView vulkan_image::create_image_view(vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels) {
		vk::ImageViewCreateInfo viewInfo = {};
		viewInfo.image = mImage;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vk::ImageView imageView;
		if (vulkan_context::instance().device.createImageView(&viewInfo, nullptr, &imageView) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	std::shared_ptr<vulkan_image> cgb::vulkan_image::generate_1px_image(uint8_t color_r, uint8_t color_g, uint8_t color_b)
	{
		uint8_t one_px[4];
		one_px[0] = color_r;
		one_px[1] = color_g;
		one_px[2] = color_b;
		one_px[3] = 0;
		return std::make_shared<vulkan_image>(&one_px, 1, 1, 4);
	}

	void vulkan_image::blit_image(vulkan_image* srcImage, vulkan_image* targetImage, 
		vk::ImageLayout srcInitLayout, vk::ImageLayout srcEndLayout, vk::ImageLayout targetInitLayout, vk::ImageLayout targetEndLayout,
		vk::AccessFlagBits srcInitAccFlags, vk::AccessFlagBits srcEndAccFlags, vk::AccessFlagBits targetInitAccFlags, vk::AccessFlagBits targetEndAccFlags,
		vk::PipelineStageFlagBits srcStage, vk::PipelineStageFlagBits targetStage,
		vulkan_command_buffer_manager* commandBufferManager) {

		vk::CommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;

		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
		beginInfo.pInheritanceInfo = &inheritanceInfo;
		vk::CommandBuffer commandBuffer = commandBufferManager->get_command_buffer(vk::CommandBufferLevel::eSecondary, beginInfo);


		int32_t blitWidth = targetImage->get_width();
		int32_t blitHeight = targetImage->get_height();
		int32_t width = srcImage->get_width();
		int32_t height = srcImage->get_height();

		vk::ImageMemoryBarrier barrier = {};
		barrier.image = targetImage->get_image();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = targetImage->get_aspects();
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseMipLevel = 0;

		barrier.oldLayout = targetInitLayout;
		barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.srcAccessMask = targetInitAccFlags;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		vk::ImageMemoryBarrier srcBarrier = barrier;
		srcBarrier.image = srcImage->get_image();
		srcBarrier.oldLayout = srcInitLayout;
		srcBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		srcBarrier.srcAccessMask = srcInitAccFlags;
		srcBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		commandBuffer.pipelineBarrier(
			srcStage, vk::PipelineStageFlagBits::eTransfer, {},
			{}, {}, { barrier, srcBarrier });

		vk::ImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { width, height, 1 };
		blit.srcSubresource.aspectMask = targetImage->get_aspects();
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { blitWidth, blitHeight, 1 };
		blit.dstSubresource.aspectMask = targetImage->get_aspects();
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		commandBuffer.blitImage(
			srcImage->get_image(), vk::ImageLayout::eTransferSrcOptimal,
			targetImage->get_image(), vk::ImageLayout::eTransferDstOptimal,
			1, &blit,
			vk::Filter::eLinear);

		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = targetEndLayout;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = targetEndAccFlags;

		srcBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		srcBarrier.newLayout = srcEndLayout;
		srcBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		srcBarrier.dstAccessMask = srcEndAccFlags;

		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, targetStage, {},
			{}, {}, { barrier, srcBarrier });
	}
}