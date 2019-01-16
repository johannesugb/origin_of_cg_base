#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <chrono>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <string>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>

#include "VkRenderObject.h"
#include "VkTexture.h"
#include "VkCommandBufferManager.h"
#include "VkDrawer.h"
#include "VkImagePresenter.h"
#include "vulkan_render_queue.h"
#include "VkRenderer.h"

const int WIDTH = 800;
const int HEIGHT = 600;

const std::string MODEL_PATH = "models/chalet.obj/chalet.obj";
const std::string TEXTURE_PATH = "textures/chalet.jpg";

const std::vector<const char*> validationLayers = {
	"vk::_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	//, vk::_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME // core functionality with vulcan 1.1, not needed anymore
	, VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME // variable rate shading extension
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vk::CreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vk::DestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

class HelloTriangleApplication {
private:
	GLFWwindow * window;
	vk::SurfaceKHR surface;

	vk::Instance instance;
	vk::DebugUtilsMessengerEXT callback;
	vk::PhysicalDevice physicalDevice = nullptr; // will be distroyed automatically on destruction of vk::Instance "instance"
	vk::Device device;
	vk::Queue graphicsQueue; // automatically created and destroyed with logical device, "device"
	vk::Queue presentQueue; // automatically created and destroyed with logical device, "device"

	

	vk::RenderPass renderPass;
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout pipelineLayout;
	std::vector<vk::Framebuffer> swapChainFramebuffers;

	vk::CommandPool commandPool;
	vk::CommandPool transferCommandPool;

	vk::DescriptorPool descriptorPool;

	vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

	bool framebufferResized = false;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	vkRenderObject* renderObject;
	vkRenderObject* renderObject2;
	vkTexture* texture;
	vkCgbImage* textureImage;
	std::shared_ptr<vkCommandBufferManager> drawCommandBufferManager;
	vkCommandBufferManager* transferCommandBufferManager;
	vkDrawer* drawer;

	// render target needed for MSAA
	vkCgbImage* colorImage;
	std::unique_ptr<vkCgbImage> depthImage;
	std::shared_ptr<vkImagePresenter> imagePresenter;
	std::shared_ptr<vulkan_render_queue> mVulkanRenderQueue;
	std::unique_ptr<vkRenderer> mRenderer;

public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	void initVulkan() {
		createInstance();
		setupDebugCallback();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();

		createCommandPools();

		vkContext::instance().physicalDevice = physicalDevice;
		vkContext::instance().device = device;
		transferCommandBufferManager = new vkCommandBufferManager(transferCommandPool, graphicsQueue);

		imagePresenter = std::make_shared<vkImagePresenter>(presentQueue, surface, findQueueFamilies(physicalDevice));
		drawCommandBufferManager = std::make_shared<vkCommandBufferManager>(imagePresenter->get_swap_chain_images_count(), commandPool, graphicsQueue);
		mVulkanRenderQueue = std::make_shared<vulkan_render_queue>(graphicsQueue, drawCommandBufferManager);
		mRenderer = std::make_unique<vkRenderer>(imagePresenter, mVulkanRenderQueue, drawCommandBufferManager);

		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		drawer = new vkDrawer(drawCommandBufferManager.get(), graphicsPipeline, pipelineLayout);

		createColorResources();
		createDepthResources();
		createFramebuffers();

		createTexture();

		createDescriptorPool();

		renderObject = new vkRenderObject(imagePresenter->get_swap_chain_images_count(), verticesQuad, indicesQuad, descriptorSetLayout, descriptorPool, texture, transferCommandBufferManager);
		renderObject2 = new vkRenderObject(imagePresenter->get_swap_chain_images_count(), verticesQuad, indicesQuad, descriptorSetLayout, descriptorPool, texture, transferCommandBufferManager);

		renderObject2->update_uniform_buffer(0, 0, imagePresenter->get_swap_chain_extent());
		renderObject2->update_uniform_buffer(1, 0, imagePresenter->get_swap_chain_extent());
		renderObject2->update_uniform_buffer(2, 0, imagePresenter->get_swap_chain_extent());
		//loadModel();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}

		// wait for all commands to complete
		device.waitIdle();
	}

	void cleanup() {
		cleanupSwapChain();

		device.destroyDescriptorPool(descriptorPool);
		device.destroyDescriptorSetLayout(descriptorSetLayout);

		delete renderObject;
		delete renderObject2;
		delete texture;
		delete textureImage;
		delete transferCommandBufferManager;
		mVulkanRenderQueue.reset();
		drawCommandBufferManager.reset();

		device.destroyCommandPool(transferCommandPool);
		device.destroyCommandPool(commandPool);

		device.destroy();
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, callback, nullptr);
		}
		instance.destroySurfaceKHR(surface, nullptr);
		instance.destroy();

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "VulkanStSt", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void cleanupSwapChain() {
		delete colorImage;
		depthImage.reset();


		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			device.destroyFramebuffer(swapChainFramebuffers[i], nullptr);
		}

		device.destroyPipeline(graphicsPipeline, nullptr);
		device.destroyPipelineLayout(pipelineLayout, nullptr);
		device.destroyRenderPass(renderPass, nullptr);

		imagePresenter.reset();
		mRenderer.reset();
	}

	void recreateSwapChain() {
		int width = 0, height = 0;
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		device.waitIdle();

		cleanupSwapChain();

		imagePresenter = std::make_shared<vkImagePresenter>(presentQueue, surface, findQueueFamilies(physicalDevice));
		mRenderer = std::make_unique<vkRenderer>(imagePresenter, mVulkanRenderQueue, drawCommandBufferManager);
		createRenderPass();
		createGraphicsPipeline();
		createColorResources();
		createDepthResources();
		createFramebuffers();
	}

	void createInstance() {
		vk::ApplicationInfo appInfo = {};
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		vk::InstanceCreateInfo createInfo = {};
		createInfo.pApplicationInfo = &appInfo;

		// extensions
		auto requiredExtensions = getRequiredExtensions();

		uint32_t extensionCount = 0;
		//vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<vk::ExtensionProperties> extensions(extensionCount);
		vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "available extensions:" << std::endl;
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << std::endl;
		}

		// TODO check if required extensions are present
		createInfo.enabledExtensionCount = requiredExtensions.size();
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		// validation layers
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		// create instance
		if (vk::createInstance(&createInfo, nullptr, &instance) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vk::enumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<vk::LayerProperties> availableLayers(layerCount);
		vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(vk::debugutils _EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void setupDebugCallback() {
		if (!enableValidationLayers) return;

		vk::DebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = vk::_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = vk::_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | vk::_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | vk::_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = vk::_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | vk::_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | vk::_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &callback) != vk::_SUCCESS) {
			throw std::runtime_error("failed to set up debug callback!");
		}
	}

	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != vk::_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vk::EnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<vk::PhysicalDevice> devices(deviceCount);
		vk::EnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				msaaSamples = getMaxUsableSampleCount();
				break;
			}
		}

		if (physicalDevice == vk::_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	// important function, checks if the features we will use 
	// are supported by our physical device
	// always extend, if more features are used!
	bool isDeviceSuitable(vk::PhysicalDevice device) {
		vk::PhysicalDeviceProperties deviceProperties;
		vk::GetPhysicalDeviceProperties(device, &deviceProperties);

		QueueFamilyIndices indices = findQueueFamilies(device);
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		vk::PhysicalDeviceFeatures supportedFeatures;
		vk::GetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	bool checkDeviceExtensionSupport(vk::PhysicalDevice device) {
		uint32_t extensionCount;
		vk::EnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
		vk::EnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vk::GetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
		vk::GetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			// OPTTIONAL_TODO, prefer device with queue which supports both, graphics(drawing) and surface(presentation)
			// for improved performance

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			vk::Bool32 presentSupport = false;
			vk::GetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	// here we pick the device features we need
	// IMPORTANT! add them to the isDeviceSuitable function
	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			vk::DeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = vk::_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}


		// IMPORTANT! add them to the isDeviceSuitable function
		vk::PhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = vk::_TRUE;

		vk::DeviceCreateInfo createInfo = {};
		createInfo.sType = vk::_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vk::CreateDevice(physicalDevice, &createInfo, nullptr, &device) != vk::_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vk::GetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vk::GetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device) {
		SwapChainSupportDetails details;
		vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vk::GetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vk::GetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vk::GetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vk::GetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void createGraphicsPipeline() {
		// shaders
		auto vertShaderCode = readFile("Shader/vert.spv");
		auto fragShaderCode = readFile("Shader/frag.spv");

		vk::ShaderModule vertShaderModule;
		vk::ShaderModule fragShaderModule;

		vertShaderModule = createShaderModule(vertShaderCode);
		fragShaderModule = createShaderModule(fragShaderCode);

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = vk::_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = vk::_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = vk::_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = vk::_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// fixed/configureable functions
		auto bindingDescription = Vertex::getBindingDescription();
		auto bindingDescription2 = Vertex::getBindingDescription2();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		vk::VertexInputBindingDescription bindings[] = { bindingDescription, bindingDescription2 };

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = vk::_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 2;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindings;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = vk::_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = vk::_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = vk::_FALSE;

		vk::Viewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)imagePresenter->get_swap_chain_extent().width;
		viewport.height = (float)imagePresenter->get_swap_chain_extent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = imagePresenter->get_swap_chain_extent();

		vk::PipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = vk::_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		//vk::PipelineViewportShadingRateImageStateCreateInfoNV shadingRateImage = {};
		//viewportState.pNext = &shadingRateImage;

		vk::PipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = vk::_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = vk::_FALSE;
		rasterizer.rasterizerDiscardEnable = vk::_FALSE;
		rasterizer.polygonMode = vk::_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = vk::_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = vk::_FRONT_FACE_COUNTER_CLOCKWISE;

		rasterizer.depthBiasEnable = vk::_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = vk::_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = vk::_FALSE;
		multisampling.rasterizationSamples = msaaSamples;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = vk::_FALSE; // Optional
		multisampling.alphaToOneEnable = vk::_FALSE; // Optional

		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = vk::_COLOR_COMPONENT_R_BIT | vk::_COLOR_COMPONENT_G_BIT | vk::_COLOR_COMPONENT_B_BIT | vk::_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = vk::_FALSE;
		colorBlendAttachment.srcColorBlendFactor = vk::_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = vk::_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = vk::_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = vk::_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = vk::_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = vk::_BLEND_OP_ADD; // Optional

		vk::PipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = vk::_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = vk::_FALSE;
		colorBlending.logicOp = vk::_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		vk::DynamicState dynamicStates[] = {
			vk::_DYNAMIC_STATE_VIEWPORT,
			vk::_DYNAMIC_STATE_LINE_WIDTH
		};

		vk::PipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = vk::_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		vk::PushConstantRange pushConstantRange = {};
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushUniforms);
		pushConstantRange.stageFlags = vk::_SHADER_STAGE_VERTEX_BIT;

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = vk::_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1; // Optional
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional

		if (vk::CreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = vk::_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = vk::_TRUE;
		depthStencil.depthWriteEnable = vk::_TRUE;
		depthStencil.depthCompareOp = vk::_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = vk::_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = vk::_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

								// finally create graphics pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = vk::_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional

		pipelineInfo.layout = pipelineLayout;

		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = vk::_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		pipelineInfo.pDepthStencilState = &depthStencil;

		if (vk::CreateGraphicsPipelines(device, vk::_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != vk::_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vk::DestroyShaderModule(device, fragShaderModule, nullptr);
		vk::DestroyShaderModule(device, vertShaderModule, nullptr);

	}

	vk::ShaderModule createShaderModule(const std::vector<char>& code) {
		vk::ShaderModuleCreateInfo createInfo = {};
		createInfo.sType = vk::_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		vk::ShaderModule shaderModule;
		if (vk::CreateShaderModule(device, &createInfo, nullptr, &shaderModule) != vk::_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	void createRenderPass() {
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.format = imagePresenter->get_swap_chain_image_format();
		colorAttachment.samples = msaaSamples;
		colorAttachment.loadOp = vk::_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = vk::_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = vk::_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = vk::_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = vk::_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = vk::_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		vk::AttachmentDescription depthAttachment = {};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = msaaSamples;
		depthAttachment.loadOp = vk::_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = vk::_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = vk::_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = vk::_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = vk::_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = vk::_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		vk::AttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = vk::_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		vk::AttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.format = imagePresenter->get_swap_chain_image_format();
		colorAttachmentResolve.samples = vk::_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = vk::_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = vk::_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = vk::_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = vk::_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = vk::_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = vk::_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		vk::AttachmentReference colorAttachmentResolveRef = {};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = vk::_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		vk::SubpassDescription subpass = {};
		subpass.pipelineBindPoint = vk::_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		vk::SubpassDependency dependency = {};
		dependency.srcSubpass = vk::_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		// wait for the color attachment (the swap chain is finished with the image and the image is ready to be written to)
		dependency.srcStageMask = vk::_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = vk::_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = vk::_ACCESS_COLOR_ATTACHMENT_READ_BIT | vk::_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<vk::AttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = vk::_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vk::CreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != vk::_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void createFramebuffers() {
		swapChainFramebuffers.resize(imagePresenter->get_swap_chain_images_count());

		for (size_t i = 0; i < imagePresenter->get_swap_chain_images_count(); i++) {
			std::array<vk::ImageView, 3> attachments = {
				colorImage->getImageView(),
				depthImage->getImageView(),
				imagePresenter->get_swap_chain_image_views()[i]
			};

			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = vk::_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = imagePresenter->get_swap_chain_extent().width;
			framebufferInfo.height = imagePresenter->get_swap_chain_extent().height;
			framebufferInfo.layers = 1;

			if (vk::CreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != vk::_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void createCommandPools() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = vk::_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = vk::_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

		if (vk::CreateCommandPool(device, &poolInfo, nullptr, &commandPool) != vk::_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}

		// create command pool for data transfers 
		vk::CommandPoolCreateInfo transferPoolInfo = {};
		transferPoolInfo.sType = vk::_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		transferPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		transferPoolInfo.flags = vk::_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Optional

		if (vk::CreateCommandPool(device, &transferPoolInfo, nullptr, &transferCommandPool) != vk::_SUCCESS) {
			throw std::runtime_error("failed to create command pool for data transfers!");
		}
	}

	void drawFrame() {
		mRenderer->start_frame();
		// update states, e.g. for animation
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		renderObject->update_uniform_buffer(vk::Context::instance().currentFrame, time, imagePresenter->get_swap_chain_extent());

		// start drawing, record draw commands, etc.
		vk::Context::instance().renderPass = renderPass;
		vk::Context::instance().frameBuffer = swapChainFramebuffers[vk::Context::instance().currentFrame];

		std::vector<vk::RenderObject*> renderObjects;
		renderObjects.push_back(renderObject);
		renderObjects.push_back(renderObject2);

		mRenderer->render(renderObjects, drawer);
		mRenderer->end_frame();
	}

	void createDescriptorSetLayout() {
		vk::DescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = vk::_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = vk::_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		vk::DescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = vk::_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = vk::_SHADER_STAGE_FRAGMENT_BIT;

		vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
		std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		layoutInfo.sType = vk::_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();


		if (vk::CreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != vk::_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void createDescriptorPool() {
		std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
		poolSizes[0].type = vk::_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(imagePresenter->get_swap_chain_images_count()) * 2;
		poolSizes[1].type = vk::_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(imagePresenter->get_swap_chain_images_count()) * 2;

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = vk::_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(imagePresenter->get_swap_chain_images_count() * 2);

		if (vk::CreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != vk::_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	// image / texture

	void createTexture() {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}
		textureImage = new vk::CgbImage(transferCommandBufferManager, pixels, texWidth, texHeight, texChannels);
		texture = new vk::Texture(textureImage);

		stbi_image_free(pixels);
	}

	// load model
	void loadModel() {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

		for (const auto& shape : shapes) {
			for (const auto& shape : shapes) {
				for (const auto& index : shape.mesh.indices) {
					Vertex vertex = {};
					vertex.pos = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};

					vertex.color = { 1.0f, 1.0f, 1.0f };

					if (uniqueVertices.count(vertex) == 0) {
						uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
						vertices.push_back(vertex);
					}

					indices.push_back(uniqueVertices[vertex]);
				}
			}
		}

		renderObject = new vk::RenderObject((uint32_t)imagePresenter->get_swap_chain_images_count(), vertices, indices, descriptorSetLayout, descriptorPool, texture, transferCommandBufferManager);
	}

	// MSAA
	vk::SampleCountFlagBits getMaxUsableSampleCount() {
		vk::PhysicalDeviceProperties physicalDeviceProperties;
		vk::GetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		vk::SampleCountFlags counts = std::min(physicalDeviceProperties.limits.framebufferColorSampleCounts, physicalDeviceProperties.limits.framebufferDepthSampleCounts);
		if (counts & vk::_SAMPLE_COUNT_64_BIT) { return vk::_SAMPLE_COUNT_64_BIT; }
		if (counts & vk::_SAMPLE_COUNT_32_BIT) { return vk::_SAMPLE_COUNT_32_BIT; }
		if (counts & vk::_SAMPLE_COUNT_16_BIT) { return vk::_SAMPLE_COUNT_16_BIT; }
		if (counts & vk::_SAMPLE_COUNT_8_BIT) { return vk::_SAMPLE_COUNT_8_BIT; }
		if (counts & vk::_SAMPLE_COUNT_4_BIT) { return vk::_SAMPLE_COUNT_4_BIT; }
		if (counts & vk::_SAMPLE_COUNT_2_BIT) { return vk::_SAMPLE_COUNT_2_BIT; }

		return vk::_SAMPLE_COUNT_1_BIT;
	}

	// attachments for framebuffer (color image to render to before resolve, depth image)

	void createDepthResources() {
		vk::Format depthFormat = findDepthFormat();

		depthImage = std::make_unique<vk::CgbImage>(transferCommandBufferManager, imagePresenter->get_swap_chain_extent().width, imagePresenter->get_swap_chain_extent().height, 1, msaaSamples, depthFormat, vk::_IMAGE_TILING_OPTIMAL, vk::_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, vk::_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vk::_IMAGE_ASPECT_DEPTH_BIT);
		depthImage->transitionImageLayout(depthFormat, vk::_IMAGE_LAYOUT_UNDEFINED, vk::_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

	}

	vk::Format findDepthFormat() {
		return findSupportedFormat(
			{ vk::_FORMAT_D32_SFLOAT, vk::_FORMAT_D32_SFLOAT_S8_UINT, vk::_FORMAT_D24_UNORM_S8_UINT },
			vk::_IMAGE_TILING_OPTIMAL,
			vk::_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
		for (vk::Format format : candidates) {
			vk::FormatProperties props;
			vk::GetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == vk::_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == vk::_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}
	
	void createColorResources() {
		vk::Format colorFormat = imagePresenter->get_swap_chain_image_format();

		colorImage = new vk::CgbImage(transferCommandBufferManager, imagePresenter->get_swap_chain_extent().width, imagePresenter->get_swap_chain_extent().height, 1, msaaSamples, colorFormat, vk::_IMAGE_TILING_OPTIMAL, vk::_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | vk::_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, vk::_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vk::_IMAGE_ASPECT_COLOR_BIT);
		colorImage->transitionImageLayout(colorFormat, vk::_IMAGE_LAYOUT_UNDEFINED, vk::_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}