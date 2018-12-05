#ifdef USE_VULKAN_CONTEXT
#include "context_vulkan.h"

namespace cgb
{
	vulkan::vulkan() : generic_glfw()
	{
		
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
