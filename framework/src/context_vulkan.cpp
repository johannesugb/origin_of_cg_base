#include "cg_stdafx.h"

namespace cgb
{
	auto vulkan::assemble_validation_layers()
	{
		std::vector<const char*> supportedValidationLayers;
		std::copy_if(
			std::begin(settings::gValidationLayersToBeActivated), std::end(settings::gValidationLayersToBeActivated),
			std::back_inserter(supportedValidationLayers),
			[](auto name) {
				auto supported = is_validation_layer_supported(name);
				if (!supported) {
					LOG_WARNING(fmt::format("Validation layer '{}' is not supported by this Vulkan instance and will not be activated."));
				}
				return supported;
			});
		return supportedValidationLayers;
	}

	vulkan::vulkan() : generic_glfw()
	{
		// So it begins
		create_instance();

		// Setup debug callback and enable all validation layers configured in global settings 
		setup_vk_debug_callback();

		// The window surface needs to be created right after the instance creation, because it can actually influence the physical device selection.

		// Select the best suitable physical device which supports all requested extensions
		pick_physical_device();

		// NOTE: Vulkan-init is not finished yet!
		//       Initialization will continue when the first window (and it's surface) is created.
	}

	vulkan::~vulkan()
	{
		// Destroy logical device
		mLogicalDevice.destroy();

		// Destroy all surfaces (unfortunately, this is not supported through the C++ API, as it seems)
		for (auto& ptr_to_tpl : mSurfaces) {
			vkDestroySurfaceKHR(mInstance, static_cast<VkSurfaceKHR>(std::get<1>(*ptr_to_tpl)), nullptr);
		}
		mSurfaces.clear();

		// Unhook debug callback
#if LOG_LEVEL > 0
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(mInstance, mDebugCallbackHandle, nullptr); 
		}
#endif

		// Destroy everything
		mInstance.destroy();
	}

	window* vulkan::create_window(const window_params& pParams)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		auto wnd = generic_glfw::create_window(pParams);
		auto surface = create_surface_for_window(wnd);
		if (0u == wnd->id() && wnd->handle()) // Only do this for the first window:
		{
			// Continue Vulkan-initialization:
			create_logical_device(surface);

			get_graphics_queue();

			// Vulkan-initialization completed.
		}
		return wnd;
	}

	void vulkan::create_instance()
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
		std::vector<const char*> supportedValidationLayers = assemble_validation_layers();
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

	bool vulkan::is_validation_layer_supported(const char* pName)
	{
		auto availableLayers = vk::enumerateInstanceLayerProperties();
		return availableLayers.end() !=  std::find_if(
			std::begin(availableLayers), std::end(availableLayers), 
			[toFind = std::string(pName)](const vk::LayerProperties& e) {
				return e.layerName == toFind;
			});
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL  vulkan::vk_debug_callback(
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
		// build the final string to be displayed (could also be an empty one)
		if (typeDescription.size() > 0) {
			typeDescription = "(" + typeDescription.substr(0, typeDescription.size() - 2) + ") ";
		}

		if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			assert(pCallbackData);
			LOG_ERROR(fmt::format("Vk-callback with Id[{}|{}] and Message[{}]",
				pCallbackData->messageIdNumber,
				pCallbackData->pMessageIdName,
				pCallbackData->pMessage));
		}
		else if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			assert(pCallbackData);
			LOG_WARNING(fmt::format("Vk-callback with Id[{}|{}] and Message[{}]",
				pCallbackData->messageIdNumber,
				pCallbackData->pMessageIdName,
				pCallbackData->pMessage));
		}
		else if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			assert(pCallbackData);
			LOG_INFO(fmt::format("Vk-callback with Id[{}|{}] and Message[{}]",
				pCallbackData->messageIdNumber,
				pCallbackData->pMessageIdName,
				pCallbackData->pMessage));
		}
		else if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
			assert(pCallbackData);
			LOG_VERBOSE(fmt::format("Vk-callback with Id[{}|{}] and Message[{}]",
				pCallbackData->messageIdNumber,
				pCallbackData->pMessageIdName,
				pCallbackData->pMessage));
		}
		return VK_FALSE; 
	}

	void vulkan::setup_vk_debug_callback()
	{
		assert(mInstance);
		// Configure logging
#if LOG_LEVEL > 0
		if (settings::gValidationLayersToBeActivated.size() == 0) {
			return;
		}

		auto msgCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
			.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
#if LOG_LEVEL > 1
				| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
#if LOG_LEVEL > 2
				| vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
#if LOG_LEVEL > 3
				| vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
#endif
#endif
#endif
			)
			.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
			.setPfnUserCallback(vulkan::vk_debug_callback);

		// Hook in
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			auto result = func(
				mInstance, 
				&static_cast<VkDebugUtilsMessengerCreateInfoEXT>(msgCreateInfo), 
				nullptr, 
				&mDebugCallbackHandle);
			if (VK_SUCCESS != result) {
				throw std::runtime_error("Failed to set up debug callback via vkCreateDebugUtilsMessengerEXT");
			}
		}
		else {
			throw std::runtime_error("Failed to vkGetInstanceProcAddr for vkCreateDebugUtilsMessengerEXT.");
		}
#endif
	}

	vk::SurfaceKHR vulkan::create_surface_for_window(window* pWindow)
	{
		assert(pWindow);
		assert(pWindow->handle());
		VkSurfaceKHR surface;
		if (VK_SUCCESS != glfwCreateWindowSurface(mInstance, pWindow->handle()->mWindowHandle, nullptr, &surface)) {
			throw std::runtime_error(fmt::format("Failed to create surface for window '{}'!", pWindow->name()));
		}
		// Insert at the back and return the newly created surface
		auto ptr_to_tpl = std::make_unique<window_surface_tuple>(std::make_tuple(pWindow, vk::SurfaceKHR{ surface }));
		auto& back = mSurfaces.emplace_back(std::move(ptr_to_tpl));
		return std::get<1>(*back);
	}

	std::optional<vk::SurfaceKHR> vulkan::get_surface_for_window(window* pWindow)
	{
		assert(pWindow);
		auto pos = std::find_if(
			std::begin(mSurfaces), std::end(mSurfaces),
			[pWindow](const window_surface_tuple_ptr& ptr_to_tpl) {
				return pWindow == std::get<0>(*ptr_to_tpl);
			});
		if (pos != mSurfaces.end()) {
			return std::get<1>(**pos); // Dereference iterator to unique_ptr of tuple
		}
		return std::nullopt;
	}

	window* vulkan::get_window_for_surface(const vk::SurfaceKHR& pSurface)
	{
		auto pos = std::find_if(
			std::begin(mSurfaces), std::end(mSurfaces),
			[&pSurface](const window_surface_tuple_ptr& ptr_to_tpl) {
				return pSurface == std::get<1>(*ptr_to_tpl);
			});
		if (pos != mSurfaces.end()) {
			return std::get<0>(**pos); // Dereference iterator to unique_ptr of tuple
		}
		return nullptr;
	}

	bool vulkan::supports_all_required_extensions(const vk::PhysicalDevice& device)
	{
		bool allExtensionsSupported = true;
		if (settings::gRequiredDeviceExtensions.size() > 0) {
			// Search for each extension requested!
			for (const auto& required : settings::gRequiredDeviceExtensions) {
				auto deviceExtensions = device.enumerateDeviceExtensionProperties();
				// See if we can find the current requested extension in the array of all device extensions
				auto result = std::find_if(std::begin(deviceExtensions), std::end(deviceExtensions),
										   [required](const vk::ExtensionProperties& devext) {
											   return strcmp(required, devext.extensionName) == 0;
										   });
				if (result == std::end(deviceExtensions)) {
					// could not find the device extension
					allExtensionsSupported = false;
				}
			}
		}
		return allExtensionsSupported;
	}

	void vulkan::pick_physical_device()
	{
		assert(mInstance);
		auto devices = mInstance.enumeratePhysicalDevices();
		if (devices.size() == 0) {
			throw std::runtime_error("Failed to find GPUs with Vulkan support.");
		}
		const vk::PhysicalDevice* currentSelection = nullptr;
		uint32_t currentScore = 0; // device score
		
		// Iterate over all devices
		for (const auto& device : devices) {
			// get features and queues
			auto properties = device.getProperties();
			auto features = device.getFeatures();
			auto queueFamilyProps = device.getQueueFamilyProperties();
			// check for required features
			bool graphicsBitSet = false;
			bool computeBitSet = false;
			for (const auto& qfp : queueFamilyProps) {
				graphicsBitSet = graphicsBitSet || ((qfp.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics);
				computeBitSet = computeBitSet || ((qfp.queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute);
			}

			// TODO/INFO: Prioritizing nvidia is a bad solution, of course. 
			// It is/was useful during development, but should be replaced by some meaningful code:
			uint32_t score =
				(graphicsBitSet ? 10 : 0) +
				(computeBitSet ? 10 : 0) +
				(properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ? 10 : 0) +
				(properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu ? 5 : 0) +
				(find_case_insensitive(properties.deviceName, "nvidia", 0) != std::string::npos ? 1 : 0);

			// Check if extensions are required
			if (!supports_all_required_extensions(device)) {
				score = 0;
			}

			if (score > currentScore) {
				currentSelection = &device;
				currentScore = score;
			}	
		}

		// Handle failure:
		if (nullptr == currentSelection) {
			if (settings::gRequiredDeviceExtensions.size() > 0) {
				throw std::runtime_error("Could not find a suitable physical device, most likely because no device supported all required device extensions.");
			}
			throw std::runtime_error("Could not find a suitable physical device.");
		}

		// Handle success:
		mPhysicalDevice = *currentSelection;
	}

	auto vulkan::find_queue_families_for_criteria(std::optional<vk::QueueFlagBits> pRequiredFlags, std::optional<vk::SurfaceKHR> pSurface)
	{
		assert(mPhysicalDevice);
		// All queue families:
		auto queueFamilies = mPhysicalDevice.getQueueFamilyProperties();
		std::vector<std::tuple<uint32_t, decltype(queueFamilies)::value_type>> indexedQueueFamilies;
		std::transform(std::begin(queueFamilies), std::end(queueFamilies),
					   std::back_inserter(indexedQueueFamilies),
					   [index = uint32_t(0)](const decltype(queueFamilies)::value_type& input) mutable {
						   auto tpl = std::make_tuple(index, input);
						   index += 1;
						   return tpl;
					   });
		// Subset to which the criteria applies:
		decltype(indexedQueueFamilies) selection;
		// Select the subset
		std::copy_if(std::begin(indexedQueueFamilies), std::end(indexedQueueFamilies),
					 std::back_inserter(selection),
					 [pRequiredFlags, pSurface, this](const std::tuple<uint32_t, decltype(queueFamilies)::value_type>& tpl) {
						 bool requirements_met = true;
						 if (pRequiredFlags) {
							 requirements_met = requirements_met && ((std::get<1>(tpl).queueFlags & *pRequiredFlags) == *pRequiredFlags);
						 }
						 if (pSurface) {
							 requirements_met = requirements_met && (mPhysicalDevice.getSurfaceSupportKHR(std::get<0>(tpl), *pSurface));
						 }
						 return requirements_met;
					 });
		return selection;
	}

	void vulkan::create_logical_device(vk::SurfaceKHR pSurface)
	{
		assert(mPhysicalDevice);
		// Determine which queue families we have, i.e. what the different queue families support and what they don't
		auto familiesWithGraphicsSupport = find_queue_families_for_criteria(vk::QueueFlagBits::eGraphics, std::nullopt);
		auto familiesWithPresentSupport = find_queue_families_for_criteria(std::nullopt, pSurface);
		auto familiesWithGrahicsAndPresentSupport = find_queue_families_for_criteria(vk::QueueFlagBits::eGraphics, pSurface);
		
		const float queuePriority = 1.0f; // TODO: Is this supposed to be priority=1 always? 
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

		if (familiesWithGrahicsAndPresentSupport.size() == 0) {
			if (familiesWithPresentSupport.size() == 0 && familiesWithGraphicsSupport.size() == 0) {
				throw std::runtime_error("Unable to find queue families which support the vk::QueueFlagBits::eGraphics flag");
			}
			LOG_WARNING_EM("Funny: No queue families which support both, graphics and presentation, but found suitable (but different) families for both.");

			// Have to add two different queues, because couldn't find one which can handle both
			// 1. add the graphics queue:
			queueCreateInfos.emplace_back()
				.setQueueFamilyIndex(std::get<0>(familiesWithGraphicsSupport[0])) // TODO: For now, just the first one is selected, what to do with the others?
				.setQueueCount(1u) // The currently available drivers will only allow you to create a small number of queues for each queue family and you don't really need more than one. (see https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues)
				.setPQueuePriorities(&queuePriority);
			// 2. add the present queue:
			queueCreateInfos.emplace_back()
				.setQueueFamilyIndex(std::get<0>(familiesWithPresentSupport[0])) // TODO: For now, just the first one is selected, what to do with the others?
				.setQueueCount(1u) // The currently available drivers will only allow you to create a small number of queues for each queue family and you don't really need more than one. (see https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues)
				.setPQueuePriorities(&queuePriority);
		}
		else {
			// Found a queue which can handle both, graphics and present => add only one instead of two
			queueCreateInfos.emplace_back()
				.setQueueFamilyIndex(std::get<0>(familiesWithGrahicsAndPresentSupport[0])) // TODO: For now, just the first one is selected, what to do with the others?
				.setQueueCount(1u) // The currently available drivers will only allow you to create a small number of queues for each queue family and you don't really need more than one. (see https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues)
				.setPQueuePriorities(&queuePriority);
		}

		// Get the same validation layers as for the instance!
		std::vector<const char*> supportedValidationLayers = assemble_validation_layers();

		auto deviceFeatures = vk::PhysicalDeviceFeatures();
		auto deviceCreateInfo = vk::DeviceCreateInfo()
			.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
			.setPQueueCreateInfos(queueCreateInfos.data())
			.setPEnabledFeatures(&deviceFeatures)
			// Whether the device supports these extensions has already been checked during device selection in @ref pick_physical_device
			// TODO: Are these the correct extensions to set here?
			.setEnabledExtensionCount(static_cast<uint32_t>(settings::gRequiredDeviceExtensions.size()))
			.setPpEnabledExtensionNames(settings::gRequiredDeviceExtensions.data())
			.setEnabledLayerCount(static_cast<uint32_t>(supportedValidationLayers.size()))
			.setPpEnabledLayerNames(supportedValidationLayers.data());
		mLogicalDevice = mPhysicalDevice.createDevice(deviceCreateInfo);
	}

	void vulkan::get_graphics_queue()
	{
		//assert(mLogicalDevice);
		//auto selectedFamilies = find_queue_families_with_flags(vk::QueueFlagBits::eGraphics);
		//if (selectedFamilies.size() == 0) {
		//	throw std::runtime_error("Unable to find queue families which support the vk::QueueFlagBits::eGraphics flag");
		//}

		//auto queue = mLogicalDevice.getQueue(std::get<0>(selectedFamilies[0]), 0); // TODO: Why always select the first one?
	}
}
