#pragma once
#include "vulkan_context.h"
#include "vulkan_image.h"
#include "vulkan_image_presenter.h"

namespace cgb {

	class vulkan_framebuffer
	{
	public:
		vulkan_framebuffer(vk::SampleCountFlagBits msaaSamples, std::shared_ptr<vulkan_image> colorImage,
			std::shared_ptr<vulkan_image> depthImage, vk::Format colorAttachmentFormat, std::vector<vk::ImageView> swapChainImageViews,
			uint32_t width, uint32_t height);
		~vulkan_framebuffer();

		void bake();

		vk::RenderPass get_render_pass() { return mRenderPass; }
		vk::Framebuffer get_swapchain_framebuffer() { return mSwapChainFramebuffers[vulkan_context::instance().currentFrame]; }

		vk::Extent2D get_framebuffer_extent() { return mExtent; }

		std::vector<vk::ClearValue> get_clear_values() { return  mClearValues; }

		void add_color_attachment(std::shared_ptr<vulkan_image> colorImage, vk::SampleCountFlagBits msaaSamples,
			vk::ImageLayout finalLayout = vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
			vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			std::array<float, 4> clearColor = std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));

		void add_dynamic_color_attachment(std::shared_ptr<vulkan_image> colorImage, std::vector<std::shared_ptr<vulkan_image>> resolveColorImages,
			vk::ImageLayout finalLayout, vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
			vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			std::array<float, 4> clearColor = std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));

	private:
		vk::RenderPass mRenderPass;
		std::vector<vk::Framebuffer> mSwapChainFramebuffers;

		vk::SampleCountFlagBits mMsaaSamples = vk::SampleCountFlagBits::e1;
		vk::Format mColorAttachmentFormat;
		std::vector<vk::ImageView> mSwapChainImageViews;
		vk::Extent2D mExtent;

		std::shared_ptr<vulkan_image> mDepthImage;

		std::vector<vk::ClearValue> mClearValues;
		std::vector<vk::AttachmentDescription> mColorAttachments;
		std::vector<vk::AttachmentDescription> mResolveColorAttachments;
		
		std::vector<std::shared_ptr<vulkan_image>> mColorAttachmentImages;
		std::vector<std::vector<vk::ImageView>> mResolveColorAttachmentImages;

		void createRenderPass();
		void createFramebuffers();
	};

}