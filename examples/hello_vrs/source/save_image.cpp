#include "save_image.h"

#include <stb_image_write.h>

save_image::save_image() {

}

save_image::~save_image() {
	for (auto thread : threadList) {
		thread->join();
	}
}

save_image & save_image::operator=(const save_image& s)
{
	for (auto thread : threadList) {
		thread->join();
	}
	threadList.clear();

	return *this;
}

void save_image::save_image_to_file(std::shared_ptr<cgb::vulkan_image> image, vk::ImageLayout imageLayout, std::string filename)
{
	auto thread = std::make_shared<std::thread>(
		[this, image, imageLayout, filename]()
		{
			// code based on https://github.com/SaschaWillems/Vulkan/blob/master/examples/screenshot/screenshot.cpp

			bool supportsBlit = true;
			auto width = image->get_width();
			auto height = image->get_height();
			auto texChannels = image->get_tex_channels();

			// Check blit support for source and destination
			VkFormatProperties formatProps;

			// Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
			vkGetPhysicalDeviceFormatProperties(cgb::vulkan_context::instance().physicalDevice, (VkFormat)image->get_format(), &formatProps);
			if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
				std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
				supportsBlit = false;
			}

			// Check if the device supports blitting to linear images 
			vkGetPhysicalDeviceFormatProperties(cgb::vulkan_context::instance().physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);
			if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
				std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
				supportsBlit = false;
			}

			// Source for the copy is the last rendered swapchain image
			vk::Image srcImage = image->get_image();

			// Create the linear tiled destination image to copy to and to read the memory from
			// Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
			cgb::vulkan_image dstImage = cgb::vulkan_image(width, height, 1, texChannels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eLinear,
				vk::ImageUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, vk::ImageAspectFlagBits::eColor);



			// Do the actual blit from the swapchain image to our host visible destination image
			auto copyCmd = cgb::vulkan_context::instance().transferCommandBufferManager->begin_single_time_commands();

			// Transition destination image to transfer destination layout
			vk::ImageMemoryBarrier imgMemBarrier = {};
			imgMemBarrier.srcAccessMask = {};
			imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			imgMemBarrier.oldLayout = vk::ImageLayout::eUndefined;
			imgMemBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
			imgMemBarrier.image = dstImage.get_image();
			imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			imgMemBarrier.subresourceRange.baseArrayLayer = 0;
			imgMemBarrier.subresourceRange.layerCount = 1;
			imgMemBarrier.subresourceRange.levelCount = 1;
			copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imgMemBarrier);


			// Transition swapchain image from present to transfer source layout
			imgMemBarrier = {};
			imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
			imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			imgMemBarrier.oldLayout = imageLayout;
			imgMemBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			imgMemBarrier.image = srcImage;
			imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			imgMemBarrier.subresourceRange.baseArrayLayer = 0;
			imgMemBarrier.subresourceRange.layerCount = 1;
			imgMemBarrier.subresourceRange.levelCount = 1;
			copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imgMemBarrier);


			// If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
			if (supportsBlit)
			{
				// Define the region to blit (we will blit the whole image)
				vk::ImageBlit blit = {};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { static_cast<int32_t>(width), static_cast<int32_t>(height), 1 };
				blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				blit.srcSubresource.mipLevel = 0;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { static_cast<int32_t>(width), static_cast<int32_t>(height), 1 };
				blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				blit.dstSubresource.mipLevel = 0;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				copyCmd.blitImage(
					srcImage, vk::ImageLayout::eTransferSrcOptimal,
					dstImage.get_image(), vk::ImageLayout::eTransferDstOptimal,
					1, &blit,
					vk::Filter::eNearest);
			}
			else
			{
				// Otherwise use image copy (requires us to manually flip components)
				VkImageCopy imageCopyRegion{};
				imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageCopyRegion.srcSubresource.layerCount = 1;
				imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageCopyRegion.dstSubresource.layerCount = 1;
				imageCopyRegion.extent.width = width;
				imageCopyRegion.extent.height = height;
				imageCopyRegion.extent.depth = 1;

				// Issue the copy command
				vkCmdCopyImage(
					copyCmd,
					srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					dstImage.get_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&imageCopyRegion);
			}



			// Transition destination image to general layout, which is the required layout for mapping the image memory later on
			imgMemBarrier = {};
			imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
			imgMemBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			imgMemBarrier.newLayout = vk::ImageLayout::eGeneral;
			imgMemBarrier.image = dstImage.get_image();
			imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			imgMemBarrier.subresourceRange.baseArrayLayer = 0;
			imgMemBarrier.subresourceRange.layerCount = 1;
			imgMemBarrier.subresourceRange.levelCount = 1;
			copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imgMemBarrier);

			// Transition back the swap chain image after the blit is done
			imgMemBarrier = {};
			imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
			imgMemBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			imgMemBarrier.newLayout = imageLayout;
			imgMemBarrier.image = srcImage;
			imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			imgMemBarrier.subresourceRange.baseArrayLayer = 0;
			imgMemBarrier.subresourceRange.layerCount = 1;
			imgMemBarrier.subresourceRange.levelCount = 1;
			copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, imgMemBarrier);

			cgb::vulkan_context::instance().transferCommandBufferManager->end_single_time_commands(copyCmd);

			// Get layout of the image (including row pitch)
			VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
			VkSubresourceLayout subResourceLayout;
			vkGetImageSubresourceLayout(cgb::vulkan_context::instance().device, dstImage.get_image(), &subResource, &subResourceLayout);

			// Map image memory so we can start copying from it
			const char* data;
			vkMapMemory(cgb::vulkan_context::instance().device, dstImage.get_image_memory().memory, 0, VK_WHOLE_SIZE, 0, (void**)& data);
			data += subResourceLayout.offset;

			std::ofstream file(filename + ".ppm", std::ios::out | std::ios::binary);

			// ppm header
			file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";

			// If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
			bool colorSwizzle = false;
			// Check if source is BGR 
			// Note: Not complete, only contains most common and basic BGR surface formats for demonstation purposes
			if (!supportsBlit)
			{
				std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
				colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), (VkFormat)image->get_format()) != formatsBGR.end());
			}

			// ppm binary pixel data
			for (uint32_t y = 0; y < height; y++)
			{
				unsigned int* row = (unsigned int*)data;
				for (uint32_t x = 0; x < width; x++)
				{
					if (colorSwizzle)
					{
						file.write((char*)row + 2, 1);
						file.write((char*)row + 1, 1);
						file.write((char*)row, 1);
					}
					else
					{
						file.write((char*)row, 3);
					}
					row++;
				}
				data += subResourceLayout.rowPitch;
			}
			file.close();



			//stbi_write_png((filename + ".png").c_str(), image->get_width(), image->get_height(), image->get_tex_channels(), data, image->get_width() * image->get_tex_channels());
			std::cout << "Screenshot saved to disk" << std::endl;

			// Clean up resources
			vkUnmapMemory(cgb::vulkan_context::instance().device, dstImage.get_image_memory().memory);
		});
		threadList.push_back(thread);
}