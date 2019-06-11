#include "vulkan_image_presenter.h"

#include <chrono>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace cgb {
	vulkan_image_presenter::vulkan_image_presenter(vk::Queue &presentQueue, vk::SurfaceKHR surface, QueueFamilyIndices queueFamilyIndices) :
		mPresentQueue(presentQueue), mSurface(surface), mQueueFamilyIndices(queueFamilyIndices)
	{
		mSwapChainRecreated = false;
		create_swap_chain();
	}


	vulkan_image_presenter::~vulkan_image_presenter()
	{
		cleanup();
	}

	void vulkan_image_presenter::cleanup() {
		vkDestroySwapchainKHR(vulkan_context::instance().device, mSwapChain, nullptr);
	}

	void vulkan_image_presenter::recreate_swapchain() {
		int width = 0, height = 0;
		while (width == 0 || height == 0) {
			GLFWwindow* window = mWindow.get();
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(vulkan_context::instance().device);

		cleanup();
		create_swap_chain();
	}

	void vulkan_image_presenter::fetch_next_swapchain_image(vk::Fence inFlightFence, vk::Semaphore signalSemaphore) {
		vulkan_context::instance().device.waitForFences(1, &inFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

		uint32_t imageIndex;
		vk::Result result = vulkan_context::instance().device.acquireNextImageKHR(mSwapChain, std::numeric_limits<uint64_t>::max(), signalSemaphore, nullptr, &imageIndex);

		if (result == vk::Result::eErrorOutOfDateKHR) {
			recreate_swapchain();
			mSwapChainRecreated = true;
			return;
		}
		else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		vulkan_context::instance().currentFrame = std::min(imageIndex, (uint32_t)vulkan_context::instance().dynamicRessourceCount - 1);
		vulkan_context::instance().currentSwapChainIndex = imageIndex;
		mImageIndex = imageIndex;

	}

	void vulkan_image_presenter::present_image(std::vector<vk::Semaphore> waitSemaphores) {
		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = waitSemaphores.size();
		presentInfo.pWaitSemaphores = waitSemaphores.data();

		vk::SwapchainKHR swapChains[] = { mSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &mImageIndex;
		presentInfo.pResults = nullptr; // Optional

		vk::Result result = mPresentQueue.presentKHR(&presentInfo);

		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
			mSwapChainRecreated = true;
			recreate_swapchain();
		}
		else if (result != vk::Result::eSuccess) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		mOldImageIndex = mImageIndex;
	}

	// swap chain create functions

	vk::SurfaceFormatKHR vulkan_image_presenter::choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
		if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
			return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
		}

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	vk::PresentModeKHR vulkan_image_presenter::choose_swap_present_mode(const std::vector<vk::PresentModeKHR> availablePresentModes) {
		vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
				return availablePresentMode;
			}
			else if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
				bestMode = availablePresentMode;
			}
		}

		return bestMode;
	}

	vk::Extent2D vulkan_image_presenter::choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities) {
		// if window manager (GLFW) is flexible with the resolution, the extends of the capabilities are std::numeric_limits<uint32_t>::max()
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			GLFWwindow* window = mWindow.get();
			glfwGetFramebufferSize(window, &width, &height);

			vk::Extent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void vulkan_image_presenter::create_swap_chain() {
		SwapChainSupportDetails swapChainSupport = vulkan_context::instance().querySwapChainSupport();

		vk::SurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats);
		vk::PresentModeKHR presentMode = choose_swap_present_mode(swapChainSupport.presentModes);
		vk::Extent2D extent = choose_swap_extent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo = {};
		createInfo.surface = mSurface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		QueueFamilyIndices indices = mQueueFamilyIndices;

		if (indices.graphicsFamily != indices.presentFamily) {
			uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = nullptr;

		if (vulkan_context::instance().device.createSwapchainKHR(&createInfo, nullptr, &mSwapChain) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create swap chain!");
		}

		mSwapChainImages = vulkan_context::instance().device.getSwapchainImagesKHR(mSwapChain);

		mSwapChainImageFormat = surfaceFormat.format;
		mSwapChainExtent = extent;

		mVulkanSwapChainImages.resize(mSwapChainImages.size());

		for (uint32_t i = 0; i < mSwapChainImages.size(); i++) {
			mVulkanSwapChainImages[i] = std::make_shared<vulkan_image>(mSwapChainImages[i], mSwapChainExtent.width, mSwapChainExtent.height, 1, 4,
				vk::SampleCountFlagBits::e1, mSwapChainImageFormat, vk::ImageUsageFlagBits::eColorAttachment, vk::ImageAspectFlagBits::eColor);
		}
	}

}