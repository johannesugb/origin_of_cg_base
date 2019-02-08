#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include <optional>
#include <memory>
#include <iostream>

class vulkan_framebuffer;
class vkMemoryManager;

struct SwapChainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};

const int MAX_FRAMES_IN_FLIGHT = 2;

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
	}
};

class vkContext
{
public:
	vkContext();
	~vkContext();

	void initVulkan(GLFWwindow * window);

	QueueFamilyIndices findQueueFamilies() { return findQueueFamilies(physicalDevice); }
	SwapChainSupportDetails querySwapChainSupport() { return querySwapChainSupport(physicalDevice); }

	vk::Instance vkInstance;
	VkDebugUtilsMessengerEXT callback;
	vk::PhysicalDevice physicalDevice = nullptr; // will be distroyed automatically on destruction of vk::Instance "instance"
	vk::Device device;
	vk::Queue graphicsQueue; // automatically created and destroyed with logical device, "device"
	vk::Queue presentQueue; // automatically created and destroyed with logical device, "device"
	vk::Queue computeQueue; // automatically created and destroyed with logical device, "device"
	vk::SurfaceKHR surface;
	vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

	vkMemoryManager* memoryManager;

	std::shared_ptr<vulkan_framebuffer> vulkanFramebuffer;

	// updated per frame/pass
	int currentFrame;

	bool shadingRateImageSupported;
	VkPhysicalDeviceShadingRateImagePropertiesNV shadingRateImageProperties;

	static vkContext& instance()
	{
		static vkContext _instance;

		return _instance;
	}

private:

	void createLogicalDevice();

	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

	void createInstance();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void setupDebugCallback();
	void createSurface(GLFWwindow * window);
	void pickPhysicalDevice();
	bool isDeviceSuitable(vk::PhysicalDevice device);
	bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
	vk::SampleCountFlagBits getMaxUsableSampleCount();


	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
};

