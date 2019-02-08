#include "vkContext.h"

#include <set>


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// TODO maybe define as paramter for context
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	, VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME // variable rate shading extension
};

// TODO maybe define as paramter for context
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

VkResult CreateDebugUtilsMessengerEXT(vk::Instance instance, const vk::DebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		const VkDebugUtilsMessengerCreateInfoEXT tmp(*pCreateInfo);
		return func(instance, &tmp, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

vkContext::vkContext()
{
}


vkContext::~vkContext()
{
	vulkanFramebuffer.reset();
	vkContext::instance().device.destroy();
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(vkInstance, callback, nullptr);
	}
	vkInstance.destroySurfaceKHR(surface, nullptr);
	vkInstance.destroy();
}


void vkContext::initVulkan(GLFWwindow * window) {

	shadingRateImageSupported = std::find(deviceExtensions.begin(), deviceExtensions.end(), VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME) != deviceExtensions.end();

	createInstance();
	setupDebugCallback();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
}

void vkContext::createInstance() {
	vk::ApplicationInfo appInfo = {};
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

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
	if (vk::createInstance(&createInfo, nullptr, &vkInstance) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create instance!");
	}
}

bool vkContext::checkValidationLayerSupport() {
	std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

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

std::vector<const char*> vkContext::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

	return extensions;
}

void vkContext::setupDebugCallback() {
	if (!enableValidationLayers) return;

	vk::DebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional

	if (CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug callback!");
	}
}

void vkContext::createSurface(GLFWwindow * window) {
	VkSurfaceKHR tempSurface;
	if (glfwCreateWindowSurface(vkInstance, window, nullptr, &tempSurface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
	surface = vk::SurfaceKHR(tempSurface);
}

void vkContext::pickPhysicalDevice() {
	std::vector<vk::PhysicalDevice> devices = vkInstance.enumeratePhysicalDevices();
	if (devices.size() == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}


	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			msaaSamples = getMaxUsableSampleCount();
			break;
		}
	}

	if (!physicalDevice) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

// important function, checks if the features we will use 
// are supported by our physical device
// always extend, if more features are used!
bool vkContext::isDeviceSuitable(vk::PhysicalDevice device) {
	vk::PhysicalDeviceProperties deviceProperties;
	device.getProperties(&deviceProperties);

	QueueFamilyIndices indices = findQueueFamilies(device);
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	vk::PhysicalDeviceFeatures2 supportedExtFeatures;

	vk::PhysicalDeviceShadingRateImageFeaturesNV shadingRateImageFeatureNV = {};
	if (shadingRateImageSupported) {
		supportedExtFeatures.pNext = &shadingRateImageFeatureNV;
	}
	device.getFeatures2(&supportedExtFeatures);
	shadingRateImageSupported = shadingRateImageFeatureNV.shadingRateImage && shadingRateImageFeatureNV.shadingRateCoarseSampleOrder;

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedExtFeatures.features.samplerAnisotropy && supportedExtFeatures.features.shaderStorageImageExtendedFormats;
}

bool vkContext::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
	std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices vkContext::findQueueFamilies(vk::PhysicalDevice device) {
	QueueFamilyIndices indices;

	std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		// OPTTIONAL_TODO, prefer device with queue which supports both, graphics(drawing) and surface(presentation)
		// for improved performance

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
			indices.graphicsFamily = i;
		}

		vk::Bool32 presentSupport = false;
		device.getSurfaceSupportKHR(i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
			indices.computeFamily = i;
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
void vkContext::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.computeFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}


	// IMPORTANT! add them to the isDeviceSuitable function
	vk::PhysicalDeviceFeatures2 deviceFeatures = {};
	deviceFeatures.features.samplerAnisotropy = VK_TRUE;
	deviceFeatures.features.shaderStorageImageExtendedFormats = VK_TRUE;

	vk::DeviceCreateInfo createInfo = {};

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	//createInfo.pEnabledFeatures = &deviceFeatures1;
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

	vk::PhysicalDeviceShadingRateImageFeaturesNV shadingRateImageFeatureNV = {};
	deviceFeatures.pNext = &shadingRateImageFeatureNV;
	if (shadingRateImageSupported) {
		shadingRateImageFeatureNV.shadingRateImage = VK_TRUE;
		shadingRateImageFeatureNV.shadingRateCoarseSampleOrder = VK_TRUE;
	}
	createInfo.pNext = &deviceFeatures;

	if (physicalDevice.createDevice(&createInfo, nullptr, &device) != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create logical device!");
	}

	device.getQueue(indices.graphicsFamily.value(), 0, &graphicsQueue);
	device.getQueue(indices.presentFamily.value(), 0, &presentQueue);
	device.getQueue(indices.computeFamily.value(), 0, &computeQueue);
}

SwapChainSupportDetails vkContext::querySwapChainSupport(vk::PhysicalDevice device) {
	SwapChainSupportDetails details;

	details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
	details.formats = device.getSurfaceFormatsKHR(surface);
	details.presentModes = device.getSurfacePresentModesKHR(surface);

	return details;
}

// MSAA
vk::SampleCountFlagBits vkContext::getMaxUsableSampleCount() {

	vk::PhysicalDeviceProperties2 physicalDeviceProperties2;

	vk::PhysicalDeviceShadingRateImagePropertiesNV shadingRateImagePropertiesNV;
	physicalDeviceProperties2.pNext = &shadingRateImagePropertiesNV;
	physicalDevice.getProperties2(&physicalDeviceProperties2);
	vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDeviceProperties2.properties;

	shadingRateImageProperties = shadingRateImagePropertiesNV;

	auto depthSampleCount = false;
	auto colorSampleCount = false;


	depthSampleCount = depthSampleCount || physicalDeviceProperties.limits.framebufferDepthSampleCounts & vk::SampleCountFlagBits::e64;
	colorSampleCount = colorSampleCount || physicalDeviceProperties.limits.framebufferColorSampleCounts & vk::SampleCountFlagBits::e64;
	if (depthSampleCount && colorSampleCount) {
		return vk::SampleCountFlagBits::e64;
	}
	depthSampleCount = depthSampleCount || physicalDeviceProperties.limits.framebufferDepthSampleCounts & vk::SampleCountFlagBits::e32;
	colorSampleCount = colorSampleCount || physicalDeviceProperties.limits.framebufferColorSampleCounts & vk::SampleCountFlagBits::e32;
	if (depthSampleCount && colorSampleCount) {
		return vk::SampleCountFlagBits::e32;
	}
	depthSampleCount = depthSampleCount || physicalDeviceProperties.limits.framebufferDepthSampleCounts & vk::SampleCountFlagBits::e16;
	colorSampleCount = colorSampleCount || physicalDeviceProperties.limits.framebufferColorSampleCounts & vk::SampleCountFlagBits::e16;
	if (depthSampleCount && colorSampleCount) {
		return vk::SampleCountFlagBits::e16;
	}
	depthSampleCount = depthSampleCount || physicalDeviceProperties.limits.framebufferDepthSampleCounts & vk::SampleCountFlagBits::e8;
	colorSampleCount = colorSampleCount || physicalDeviceProperties.limits.framebufferColorSampleCounts & vk::SampleCountFlagBits::e8;
	if (depthSampleCount && colorSampleCount) {
		return vk::SampleCountFlagBits::e8;
	}
	depthSampleCount = depthSampleCount || physicalDeviceProperties.limits.framebufferDepthSampleCounts & vk::SampleCountFlagBits::e4;
	colorSampleCount = colorSampleCount || physicalDeviceProperties.limits.framebufferColorSampleCounts & vk::SampleCountFlagBits::e4;
	if (depthSampleCount && colorSampleCount) {
		return vk::SampleCountFlagBits::e4;
	}
	depthSampleCount = depthSampleCount || physicalDeviceProperties.limits.framebufferDepthSampleCounts & vk::SampleCountFlagBits::e2;
	colorSampleCount = colorSampleCount || physicalDeviceProperties.limits.framebufferColorSampleCounts & vk::SampleCountFlagBits::e2;
	if (depthSampleCount && colorSampleCount) {
		return vk::SampleCountFlagBits::e2;
	}
	//vk::SampleCountFlags counts = std::min(physicalDeviceProperties.limits.framebufferColorSampleCounts, physicalDeviceProperties.limits.framebufferDepthSampleCounts);
	//if (counts & vk::SampleCountFlagBits::e64 ) { return vk::_SAMPLE_COUNT_64_BIT; }
	//if (counts & vk::SampleCountFlagBits::e32) { return vk::_SAMPLE_COUNT_32_BIT; }
	//if (counts & vk::SampleCountFlagBits::e16) { return vk::_SAMPLE_COUNT_16_BIT; }
	//if (counts & vk::SampleCountFlagBits::e8) { return vk::_SAMPLE_COUNT_8_BIT; }
	//if (counts & vk::SampleCountFlagBits::e4) { return vk::_SAMPLE_COUNT_4_BIT; }
	//if (counts & vk::SampleCountFlagBits::e2) { return vk::_SAMPLE_COUNT_2_BIT; }

	return vk::SampleCountFlagBits::e1;
}