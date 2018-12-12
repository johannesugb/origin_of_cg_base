#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class vkMemoryManager;

class vkContext
{
public:
	vkContext();
	~vkContext();

	VkInstance vkInstance;
	VkDebugUtilsMessengerEXT callback;
	VkPhysicalDevice physicalDevice;
	VkDevice device;

	VkQueue graphicsQueue; // automatically created and destroyed with logical device, "device"
	VkQueue presentQueue; // automatically created and destroyed with logical device, "device"

	// TODO outsource to Command Manager
	VkCommandPool transferCommandPool;
	VkCommandPool commandPool;

	vkMemoryManager* memoryManager;

	VkRenderPass renderPass;

	// updated per frame/pass
	VkFramebuffer frameBuffer;
	int currentFrame;

	static vkContext& instance()
	{
		static vkContext _instance;

		return _instance;
	}
};

