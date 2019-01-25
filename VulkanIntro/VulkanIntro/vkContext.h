#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include <optional>
#include <memory>

class vulkan_framebuffer;
class vkMemoryManager;


const int MAX_FRAMES_IN_FLIGHT = 2;

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class vkContext
{
public:
	vkContext();
	~vkContext();

	vk::Instance vkInstance;
	vk::DebugUtilsMessengerEXT callback;
	vk::PhysicalDevice physicalDevice;
	vk::Device device;

	vkMemoryManager* memoryManager;

	std::shared_ptr<vulkan_framebuffer> vulkanFramebuffer;

	// updated per frame/pass
	int currentFrame;

	static vkContext& instance()
	{
		static vkContext _instance;

		return _instance;
	}
};

