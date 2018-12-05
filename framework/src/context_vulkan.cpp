#ifdef USE_VULKAN_CONTEXT
#include "context_vulkan.h"

namespace cgb
{
	vulkan::vulkan() : generic_glfw()
	{
		// Information about the application for the instance creation call
		auto appInfo = vk::ApplicationInfo(sApplicationName.c_str(), sApplicationVersion, 
			"cg_base", VK_MAKE_VERSION(0, 1, 0), // TODO: Real version of cg_base
			VK_API_VERSION_1_1);

		// GLFW requires several extensions to interface with the window system. Query them.
		uint32_t glfwExtensionCount = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> requiredExtensions;
		requiredExtensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);
		for (auto i=0; i < glfwExtensionCount; ++i)
		{
			requiredExtensions.push_back(glfwExtensions[i]);
		}


		auto extensions = vk::enumerateInstanceExtensionProperties();
		for (const auto& extension : extensions)
		{
			std::cout << extension.extensionName << std::endl;
		}


		// Gather all previously prepared info for instance creation and put in one struct:
		auto instCreateInfo = vk::InstanceCreateInfo()
			.setPApplicationInfo(&appInfo)
			.setEnabledExtensionCount(requiredExtensions.size())
			.setPpEnabledExtensionNames(requiredExtensions.data())
			.setEnabledLayerCount(0u);
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

}

#endif // USE_VULKAN_CONTEXT
