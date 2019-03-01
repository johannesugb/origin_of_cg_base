#pragma once
#include "vulkan_context.h"
#include "vulkan_image.h"
#include "vulkan_image_presenter.h"

namespace cgb {

	class vulkan_framebuffer
	{
	public:
		vulkan_framebuffer(vk::SampleCountFlagBits msaaSamples, std::shared_ptr<vulkan_image> colorImage,
			std::shared_ptr<vulkan_image> depthImage, std::shared_ptr<vulkan_image_presenter> imagePresenter);
		~vulkan_framebuffer();

		vk::RenderPass get_render_pass() { return renderPass; }
		vk::Framebuffer get_swapchain_framebuffer() { return swapChainFramebuffers[vulkan_context::instance().currentFrame]; }

		vk::Extent2D get_framebuffer_extent() { return imagePresenter->get_swap_chain_extent(); }

	private:
		vk::RenderPass renderPass;
		std::vector<vk::Framebuffer> swapChainFramebuffers;

		vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
		std::shared_ptr<vulkan_image> colorImage;
		std::shared_ptr<vulkan_image> depthImage;
		std::shared_ptr<vulkan_image_presenter> imagePresenter;

		void createRenderPass();
		void createFramebuffers();
	};

}