#include "cg_stdafx.h"

namespace cgb
{
	vulkan::vulkan() : generic_glfw()
	{
		// Information about the application for the instance creation call
		auto appInfo = vk::ApplicationInfo(settings::gApplicationName.c_str(), settings::gApplicationVersion,
			"cg_base", VK_MAKE_VERSION(0, 1, 0), // TODO: Real version of cg_base
			VK_API_VERSION_1_1);

		// GLFW requires several extensions to interface with the window system. Query them.
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> requiredExtensions;
		requiredExtensions.assign(glfwExtensions, static_cast<const char**>(glfwExtensions + glfwExtensionCount));
		requiredExtensions.insert(
			std::end(requiredExtensions), 
			std::begin(settings::gRequiredInstanceExtensions), std::end(settings::gRequiredInstanceExtensions));

		// Check for each validation layer if it exists and activate all which do.
		std::vector<const char*> supportedValidationLayers;
		std::copy_if(
			std::begin(settings::gValidationLayersToBeActivated), std::end(settings::gValidationLayersToBeActivated),
			std::back_inserter(supportedValidationLayers),
			[](auto name) {
				return is_validation_layer_supported(name);
			});
		// Enable extension to receive callbacks for the validation layers
		if (supportedValidationLayers.size() > 0) {
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// Gather all previously prepared info for instance creation and put in one struct:
		auto instCreateInfo = vk::InstanceCreateInfo()
			.setPApplicationInfo(&appInfo)
			.setEnabledExtensionCount(static_cast<uint32_t>(requiredExtensions.size()))
			.setPpEnabledExtensionNames(requiredExtensions.data())
			.setEnabledLayerCount(static_cast<uint32_t>(supportedValidationLayers.size()))
			.setPpEnabledLayerNames(supportedValidationLayers.data());
		// Create it, errors will result in an exception.
		mInstance = vk::createInstance(instCreateInfo);
	}

	window vulkan::create_window(const window_params& pParams)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		auto wnd = generic_glfw::create_window(pParams);

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::cout << extensionCount << " extensions supported" << std::endl;

		glm::mat4 matrix;
		glm::vec4 vec;
		auto test = matrix * vec;

		return wnd;
	}

	bool vulkan::is_validation_layer_supported(const char* pName)
	{
		auto availableLayers = vk::enumerateInstanceLayerProperties();
		return availableLayers.end() !=  std::find_if(
			std::begin(availableLayers), std::end(availableLayers), 
			[toFind = std::string(pName)](const auto& e) {
				return e.layerName == toFind;
			});
	}

	VkBool32 vulkan::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT pMessageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT pMessageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		// build a string from the message type parameter
		std::string typeDescription;
		if ((pMessageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) != 0) {
			typeDescription += "General, ";
		}
		if ((pMessageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0) {
			typeDescription += "Validation, ";
		}
		if ((pMessageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
			typeDescription += "Performance, ";
		}
		if (typeDescription.size() > 0) {
			typeDescription = "(" + typeDescription.substr(0, typeDescription.size() - 2) + ") ";
		}

		if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			LOG_ERROR("asdf");
		}
		else if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {

		}
		else if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {

		}
		else if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {

		}
		return 0; // TODO: retturn what?
	}
}
