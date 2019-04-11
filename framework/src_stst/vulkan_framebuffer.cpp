#include "vulkan_framebuffer.h"

namespace cgb {

	vulkan_framebuffer::vulkan_framebuffer(vk::SampleCountFlagBits msaaSamples, std::shared_ptr<vulkan_image> colorImage,
		std::shared_ptr<vulkan_image> depthImage, vk::Format colorAttachmentFormat, std::vector<vk::ImageView> swapChainImageViews,
		uint32_t width, uint32_t height) :
		mMsaaSamples(msaaSamples), mDepthImage(depthImage), mColorAttachmentFormat(colorAttachmentFormat),
		mSwapChainImageViews(swapChainImageViews)
	{
		mExtent.width = width;
		mExtent.height = height;

		mClearValues = std::vector<vk::ClearValue>(2);
		mClearValues[0].color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
		mClearValues[1].depthStencil = { 1.0f, 0 };

		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.format = colorImage->get_format();
		colorAttachment.samples = mMsaaSamples;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
		mColorAttachments.push_back(colorAttachment);

		vk::AttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.format = mColorAttachmentFormat;
		colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
		colorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachmentResolve.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachmentResolve.finalLayout = vk::ImageLayout::ePresentSrcKHR;
		mResolveColorAttachments.push_back(colorAttachmentResolve);

		mColorAttachmentImages.push_back(colorImage);
		mResolveColorAttachmentImages.push_back(swapChainImageViews);
	}


	vulkan_framebuffer::~vulkan_framebuffer()
	{
		for (size_t i = 0; i < mSwapChainFramebuffers.size(); i++) {
			vulkan_context::instance().device.destroyFramebuffer(mSwapChainFramebuffers[i], nullptr);
		}
		vulkan_context::instance().device.destroyRenderPass(mRenderPass, nullptr);
	}

	void cgb::vulkan_framebuffer::bake()
	{
		createRenderPass();
		createFramebuffers();
	}

	void cgb::vulkan_framebuffer::add_color_attachment(std::shared_ptr<vulkan_image> colorImage, vk::SampleCountFlagBits msaaSamples, 
		vk::ImageLayout finalLayout, vk::ImageLayout initialLayout, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp, 
		vk::AttachmentLoadOp stencilLoadOp, vk::AttachmentStoreOp stencilStoreOp, std::array<float, 4> clearColor)
	{
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.format = colorImage->get_format();
		colorAttachment.samples = colorImage->get_num_samples();
		colorAttachment.loadOp = loadOp;
		colorAttachment.storeOp = storeOp;
		colorAttachment.stencilLoadOp = stencilLoadOp;
		colorAttachment.stencilStoreOp = stencilStoreOp;
		colorAttachment.initialLayout = initialLayout;
		colorAttachment.finalLayout = finalLayout;
		mColorAttachments.push_back(colorAttachment);

		vk::ClearValue clearValue = {};
		clearValue.color = clearColor;

		if (mClearValues.size() > 0) {
			auto depthClearVal = mClearValues[mClearValues.size() - 1];
			mClearValues.pop_back();
			mClearValues.push_back(clearValue);
			mClearValues.push_back(depthClearVal);
		}
		else {
			mClearValues.push_back(clearValue);
		}

		mColorAttachmentImages.push_back(colorImage);
	}

	void cgb::vulkan_framebuffer::add_dynamic_color_attachment(std::shared_ptr<vulkan_image> colorImage, std::vector<std::shared_ptr<vulkan_image>> resolveColorImages, 
		vk::ImageLayout finalLayout, vk::ImageLayout initialLayout, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp, 
		vk::AttachmentLoadOp stencilLoadOp, vk::AttachmentStoreOp stencilStoreOp, std::array<float, 4> clearColor)
	{
		assert(resolveColorImages.size() == vulkan_context::instance().dynamicRessourceCount);

		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.format = colorImage->get_format();
		colorAttachment.samples = colorImage->get_num_samples();
		colorAttachment.loadOp = loadOp;
		colorAttachment.storeOp = storeOp;
		colorAttachment.stencilLoadOp = stencilLoadOp;
		colorAttachment.stencilStoreOp = stencilStoreOp;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
		mColorAttachments.push_back(colorAttachment);

		vk::AttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.format = colorImage->get_format();
		colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
		colorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachmentResolve.initialLayout = initialLayout;
		colorAttachmentResolve.finalLayout = finalLayout;
		mResolveColorAttachments.push_back(colorAttachmentResolve);

		mColorAttachmentImages.push_back(colorImage);
		std::vector<vk::ImageView> imageViews(resolveColorImages.size());
		for (int i = 0; i < resolveColorImages.size(); i++) {
			imageViews[i] = resolveColorImages[i]->get_image_view();
		}
		mResolveColorAttachmentImages.push_back(imageViews);

		vk::ClearValue clearValue = {};
		clearValue.color = clearColor;

		if (mClearValues.size() > 0) {
			auto depthClearVal = mClearValues[mClearValues.size() - 1];
			mClearValues.pop_back();
			mClearValues.push_back(clearValue);
			mClearValues.push_back(depthClearVal);
		}
		else {
			mClearValues.push_back(clearValue);
		}
	}

	void vulkan_framebuffer::createRenderPass() {
		std::vector<vk::AttachmentDescription> attachments;
		attachments.insert(attachments.end(), mColorAttachments.begin(), mColorAttachments.end());

		uint32_t colorAttachmentSize = mColorAttachments.size();
		std::vector<vk::AttachmentReference> colorAttachmentRefs(colorAttachmentSize);
		
		for (int i = 0; i < colorAttachmentRefs.size(); i++) {
			vk::AttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = i;
			colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
			colorAttachmentRefs[i] = colorAttachmentRef;
		}

		std::vector<vk::AttachmentReference> resolveColorAttachmentRefs(colorAttachmentSize);

		for (int i = 0; i < resolveColorAttachmentRefs.size(); i++) {
			vk::AttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = i + 1 + colorAttachmentSize;
			colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
			resolveColorAttachmentRefs[i] = colorAttachmentRef;
		}

		vk::AttachmentDescription depthAttachment = {};
		depthAttachment.format = mDepthImage->get_format();
		depthAttachment.samples = mMsaaSamples;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		attachments.push_back(depthAttachment);

		vk::AttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = colorAttachmentSize;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		attachments.insert(attachments.end(), mResolveColorAttachments.begin(), mResolveColorAttachments.end());

		vk::SubpassDescription subpass = {};
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = colorAttachmentRefs.size();
		subpass.pColorAttachments = colorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = resolveColorAttachmentRefs.data();

		vk::SubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		// wait for the color attachment (the swap chain is finished with the image and the image is ready to be written to)
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcAccessMask = {};

		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vulkan_context::instance().device.createRenderPass(&renderPassInfo, nullptr, &mRenderPass) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void vulkan_framebuffer::createFramebuffers() {
		mSwapChainFramebuffers.resize(mSwapChainImageViews.size());

		for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {			
			std::vector<vk::ImageView> attachments;

			for (int j = 0; j < mColorAttachmentImages.size(); j++) {
				attachments.push_back(mColorAttachmentImages[j]->get_image_view());
			}
			attachments.push_back(mDepthImage->get_image_view());
			for (int j = 0; j < mResolveColorAttachmentImages.size(); j++) {
				attachments.push_back(mResolveColorAttachmentImages[j][i]);
			}

			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.renderPass = mRenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = mExtent.width;
			framebufferInfo.height = mExtent.height;
			framebufferInfo.layers = 1;

			if (vulkan_context::instance().device.createFramebuffer(&framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != vk::Result::eSuccess) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

}