#pragma once
#include "vkContext.h"

#include <vector>
#include <memory>

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class vkImagePresenter
{
public:
	vkImagePresenter(VkQueue &presentQueue, VkSurfaceKHR surface, QueueFamilyIndices queueFamilyIndices);
	virtual ~vkImagePresenter();

	void fetch_next_swapchain_image(VkFence inFlightFence, VkSemaphore signalSemaphore);
	void present_image(std::vector<VkSemaphore> waitSemaphores);

	bool is_swap_chain_recreated() { return mSwapChainRecreated; }
	void recreate_swapchain();

	VkFormat get_swap_chain_image_format() { return mSwapChainImageFormat; }
	VkExtent2D get_swap_chain_extent() { return mSwapChainExtent; }
	size_t get_swap_chain_images_count() { return mSwapChainImages.size(); }
	std::vector<VkImageView> get_swap_chain_image_views() { return mSwapChainImageViews; }
private:
	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapChainImages; // created and destroyed with the swap chain
	std::vector<VkImageView> mSwapChainImageViews;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;

	bool mSwapChainRecreated;
	uint32_t mImageIndex;
	uint32_t mOldImageIndex;

	VkQueue mPresentQueue;
	std::shared_ptr<GLFWwindow> mWindow;
	VkSurfaceKHR mSurface;
	QueueFamilyIndices mQueueFamilyIndices;


	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities);

	void cleanup();
	void createSwapChain();
	void createImageViews();

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
};

