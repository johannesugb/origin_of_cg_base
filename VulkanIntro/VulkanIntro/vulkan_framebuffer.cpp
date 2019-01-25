#include "vulkan_framebuffer.h"



vulkan_framebuffer::vulkan_framebuffer(vk::SampleCountFlagBits msaaSamples, std::shared_ptr<vkCgbImage> colorImage, 
	std::shared_ptr<vkCgbImage> depthImage, std::shared_ptr<vkImagePresenter> imagePresenter) :
	msaaSamples(msaaSamples), colorImage(colorImage), depthImage(depthImage), imagePresenter(imagePresenter)
{
	createRenderPass();
	createFramebuffers();
}


vulkan_framebuffer::~vulkan_framebuffer()
{
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		vkContext::instance().device.destroyFramebuffer(swapChainFramebuffers[i], nullptr);
	}
	vkContext::instance().device.destroyRenderPass(renderPass, nullptr);
}

void vulkan_framebuffer::createRenderPass() {
	vk::AttachmentDescription colorAttachment = {};
	colorAttachment.format = colorImage->get_format();
	colorAttachment.samples = msaaSamples;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentDescription depthAttachment = {};
	depthAttachment.format = depthImage->get_format();
	depthAttachment.samples = msaaSamples;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = imagePresenter->get_swap_chain_image_format();
	colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
	colorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachmentResolve.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachmentResolve.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	vk::SubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	// wait for the color attachment (the swap chain is finished with the image and the image is ready to be written to)
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.srcAccessMask = {};

	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

	std::array<vk::AttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
	vk::RenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkContext::instance().device.createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void vulkan_framebuffer::createFramebuffers() {
	swapChainFramebuffers.resize(imagePresenter->get_swap_chain_images_count());

	for (size_t i = 0; i < imagePresenter->get_swap_chain_images_count(); i++) {
		std::array<vk::ImageView, 3> attachments = {
			colorImage->get_image_view(),
			depthImage->get_image_view(),
			imagePresenter->get_swap_chain_image_views()[i]
		};

		vk::FramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = imagePresenter->get_swap_chain_extent().width;
		framebufferInfo.height = imagePresenter->get_swap_chain_extent().height;
		framebufferInfo.layers = 1;

		if (vkContext::instance().device.createFramebuffer(&framebufferInfo, nullptr, &swapChainFramebuffers[i]) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}