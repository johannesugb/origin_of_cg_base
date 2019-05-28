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

		vulkan_framebuffer(uint32_t width, uint32_t height,
			size_t swapChainImageCount, vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1);
		~vulkan_framebuffer();

		void bake();

		vk::RenderPass get_render_pass() { return mRenderPass; }
		vk::Framebuffer get_swapchain_framebuffer() {

			return mSwapChainFramebuffers[vulkan_context::instance().currentSwapChainIndex];
			if (mSwapChainFramebuffers.size() < 1) {
				return nullptr;
			}
			else if (mSwapChainFramebuffers.size() == 1) {
				return mSwapChainFramebuffers[0];
			}
			else {
				if (m_is_swapchain_framebuffer) {
					return mSwapChainFramebuffers[vulkan_context::instance().currentSwapChainIndex];
				}
				else {
					return mSwapChainFramebuffers[vulkan_context::instance().currentFrame];
				}
			}
		}

		vk::Extent2D get_framebuffer_extent() { return mExtent; }

		std::vector<vk::ClearValue> get_clear_values() { return  mClearValues; }

		void add_color_attachment(std::shared_ptr<vulkan_image> colorImage, 
			vk::ImageLayout finalLayout = vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
			vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			std::array<float, 4> clearColor = std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));

		void add_dynamic_color_attachment(std::vector<std::shared_ptr<vulkan_image>> colorImages,
			vk::ImageLayout finalLayout = vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
			vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			std::array<float, 4> clearColor = std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));

		void add_dynamic_color_attachment(std::shared_ptr<vulkan_image> colorImage, std::vector<std::shared_ptr<vulkan_image>> resolveColorImages,
			vk::ImageLayout finalLayout, vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
			vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			std::array<float, 4> clearColor = std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));

		void add_swapchain_color_attachment(std::shared_ptr<vulkan_image_presenter> imagePresenter, std::shared_ptr<vulkan_image> colorImage = nullptr,
			vk::ImageLayout finalLayout = vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
			vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			std::array<float, 4> clearColor = std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));

		void set_depth_attachment(std::shared_ptr<vulkan_image> depthImage,
			vk::ImageLayout finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
			vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			float clearDepth = 1.0f, uint32_t cleaerStencil = 0);

	private:
		bool m_is_swapchain_framebuffer = false;

		vk::RenderPass mRenderPass;
		std::vector<vk::Framebuffer> mSwapChainFramebuffers;

		vk::SampleCountFlagBits mMsaaSamples = vk::SampleCountFlagBits::e1;
		vk::Format mColorAttachmentFormat;

		size_t mSwapChainImageCount;
		vk::Extent2D mExtent;

		std::shared_ptr<vulkan_image> mDepthImage;

		std::vector<vk::ClearValue> mClearValues;
		std::vector<vk::AttachmentDescription> mColorAttachments;
		vk::AttachmentDescription mDepthAttachment;
		std::vector<vk::AttachmentDescription> mResolveColorAttachments;
		
		std::vector<std::vector<vk::ImageView>> mColorAttachmentImages;
		std::vector<std::vector<vk::ImageView>> mResolveColorAttachmentImages;

		void createRenderPass();
		void createFramebuffers();
	};

}