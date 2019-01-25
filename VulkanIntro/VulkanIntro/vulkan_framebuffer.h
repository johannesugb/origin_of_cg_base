#pragma once
#include "vkContext.h"
#include "vkCgbImage.h"
#include "vkImagePresenter.h"

class vulkan_framebuffer
{
public:
	vulkan_framebuffer(vk::SampleCountFlagBits msaaSamples, std::shared_ptr<vkCgbImage> colorImage, 
		std::shared_ptr<vkCgbImage> depthImage, std::shared_ptr<vkImagePresenter> imagePresenter);
	~vulkan_framebuffer();

	vk::RenderPass get_render_pass() { return renderPass; }
	vk::Framebuffer get_swapchain_framebuffer() { return swapChainFramebuffers[vkContext::instance().currentFrame]; }

private:
	vk::RenderPass renderPass;
	std::vector<vk::Framebuffer> swapChainFramebuffers;

	vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
	std::shared_ptr<vkCgbImage> colorImage;
	std::shared_ptr<vkCgbImage> depthImage;
	std::shared_ptr<vkImagePresenter> imagePresenter;

	void createRenderPass();
	void createFramebuffers();
};

