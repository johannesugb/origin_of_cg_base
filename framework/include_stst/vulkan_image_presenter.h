#pragma once
#include "vulkan_context.h"

#include <vector>
#include <memory>
namespace cgb {
	class vulkan_image_presenter
	{
	public:
		vulkan_image_presenter(vk::Queue &presentQueue, vk::SurfaceKHR surface, QueueFamilyIndices queueFamilyIndices);
		virtual ~vulkan_image_presenter();

		void fetch_next_swapchain_image(vk::Fence inFlightFence, vk::Semaphore signalSemaphore);
		void present_image(std::vector<vk::Semaphore> waitSemaphores);

		bool is_swap_chain_recreated() { return mSwapChainRecreated; }
		void recreate_swapchain();

		vk::Format get_swap_chain_image_format() { return mSwapChainImageFormat; }
		vk::Extent2D get_swap_chain_extent() { return mSwapChainExtent; }
		size_t get_swap_chain_images_count() { return mSwapChainImages.size(); }
		std::vector<vk::ImageView> get_swap_chain_image_views() { return mSwapChainImageViews; }
	private:
		vk::SwapchainKHR mSwapChain;
		std::vector<vk::Image> mSwapChainImages; // created and destroyed with the swap chain
		std::vector<vk::ImageView> mSwapChainImageViews;
		vk::Format mSwapChainImageFormat;
		vk::Extent2D mSwapChainExtent;

		bool mSwapChainRecreated;
		uint32_t mImageIndex;
		uint32_t mOldImageIndex;

		vk::Queue mPresentQueue;
		std::shared_ptr<GLFWwindow> mWindow;
		vk::SurfaceKHR mSurface;
		QueueFamilyIndices mQueueFamilyIndices;


		vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
		vk::PresentModeKHR choose_swap_present_mode(const std::vector<vk::PresentModeKHR> availablePresentModes);
		vk::Extent2D choose_swap_extent(const vk::SurfaceCapabilitiesKHR & capabilities);

		void cleanup();
		void create_swap_chain();
		void create_image_views();

		vk::ImageView create_image_view(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
	};

}