#include "context_vulkan.h"

namespace cgb
{
	size_t vulkan::sSettingMaxFramesInFlight = 2;
	size_t vulkan::sActualMaxFramesInFlight = 2;

	std::vector<const char*> vulkan::sRequiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

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

	vulkan::vulkan()
		: generic_glfw()
		, mFrameCounter(0)
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
		// Destroy all:
		//  - swap chains,
		//  - surfaces,
		//  - and windows
		for (auto& ptrToSwapChainData : mSurfSwap) {
			// Unlike images, the image views were explicitly created by us, so we need to add a similar loop to destroy them again at the end of the program [3]
			for (auto& imageView : ptrToSwapChainData->mSwapChainImageViews) {
				mLogicalDevice.destroyImageView(imageView);
			}
			mLogicalDevice.destroySwapchainKHR(ptrToSwapChainData->mSwapChain);
			mInstance.destroySurfaceKHR(ptrToSwapChainData->mSurface);
			generic_glfw::close_window(*ptrToSwapChainData->mWindow);
		}
		mSurfSwap.clear();

		// Destroy the semaphores
		cleanup_sync_objects();

		// Destroy logical device
		mLogicalDevice.destroy();

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

	void vulkan::begin_composition()
	{ 
	}

	void vulkan::end_composition()
	{
		mLogicalDevice.waitIdle();
	}

	void vulkan::begin_frame()
	{
		mFrameCounter += 1;
	
		// Wait for the prev-prev frame (fence-ping-pong)
		// TODO: We should only wait for fences if some were submitted 
		//       ...during the last RENDER-call!!!
		auto& fence = fence_current_frame();
		mLogicalDevice.waitForFences(1u, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		mLogicalDevice.resetFences(1u, &fence);
	}

	void vulkan::end_frame()
	{
	}

	void vulkan::draw_triangle(const pipeline& pPipeline, const command_buffer& pCommandBuffer)
	{
		pCommandBuffer.mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pPipeline.mPipeline);
		pCommandBuffer.mCommandBuffer.draw(3u, 1u, 0u, 0u);
	}

	void vulkan::draw_vertices(const pipeline& pPipeline, const command_buffer& pCommandBuffer, vk::ArrayProxy<const vk::Buffer> pBuffers, uint32_t pVertexCount)
	{
		pCommandBuffer.mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pPipeline.mPipeline);
		pCommandBuffer.mCommandBuffer.bindVertexBuffers(0u, std::move(pBuffers), { 0 });
		pCommandBuffer.mCommandBuffer.draw(pVertexCount, 1u, 0u, 0u);
	}

	window* vulkan::create_window(const window_params& pWndParams, const swap_chain_params& pSwapParams)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// Create a tuple of window, surface, and swap chain
		auto wnd = generic_glfw::create_window(pWndParams, pSwapParams);
		auto surface = create_surface_for_window(wnd);
		// Vulkan init completion?
		if (0u == wnd->id() && wnd->handle()) { // We need a surface to create the logical device => do it after the first window has been created
			// This finishes Vulkan initialization:
			create_and_assign_logical_device(surface);
			// Now that we've got the logical device, get the settings parameter and create the correct number of semaphores
			sActualMaxFramesInFlight = sSettingMaxFramesInFlight;
			create_sync_objects();
		}
		// Continue tuple creation
		auto swapChain = create_swap_chain(wnd, surface, pSwapParams);

		// Insert at the back
		auto& back = mSurfSwap.emplace_back(std::make_unique<swap_chain_data>(std::move(swapChain)));
		return back->mWindow;
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

	void vulkan::create_sync_objects()
	{
		auto semaphoreInfo = vk::SemaphoreCreateInfo();
		for (auto i = 0; i < sActualMaxFramesInFlight; ++i) {
			mImageAvailableSemaphores.push_back(mLogicalDevice.createSemaphore(semaphoreInfo));
		}
		for (auto i = 0; i < sActualMaxFramesInFlight; ++i) {
			mRenderFinishedSemaphores.push_back(mLogicalDevice.createSemaphore(semaphoreInfo));
		}

		auto fenceInfo = vk::FenceCreateInfo()
			.setFlags(vk::FenceCreateFlagBits::eSignaled);
		for (auto i = 0; i < sActualMaxFramesInFlight; ++i) {
			mInFlightFences.push_back(mLogicalDevice.createFence(fenceInfo));
		}

	}

	void vulkan::cleanup_sync_objects()
	{
		for (auto& fen : mInFlightFences) {
			mLogicalDevice.destroyFence(fen);
		}
		mInFlightFences.clear();

		for (auto& sem : mRenderFinishedSemaphores) {
			mLogicalDevice.destroySemaphore(sem);
		}
		mRenderFinishedSemaphores.clear();

		for (auto& sem : mImageAvailableSemaphores) {
			mLogicalDevice.destroySemaphore(sem);
		}
		mImageAvailableSemaphores.clear();
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

	vk::SurfaceKHR vulkan::create_surface_for_window(const window* pWindow)
	{
		assert(pWindow);
		assert(pWindow->handle());
		VkSurfaceKHR surface;
		if (VK_SUCCESS != glfwCreateWindowSurface(mInstance, pWindow->handle()->mHandle, nullptr, &surface)) {
			throw std::runtime_error(fmt::format("Failed to create surface for window '{}'!", pWindow->name()));
		}
		return surface;
	}

	swap_chain_data* vulkan::get_surf_swap_tuple_for_window(const window* pWindow)
	{
		assert(pWindow);
		auto pos = std::find_if(
			std::begin(mSurfSwap), std::end(mSurfSwap),
			[pWindow](const swap_chain_data_ptr& ptr_to_tpl) {
				return pWindow == ptr_to_tpl->mWindow;
			});
		if (pos != mSurfSwap.end()) {
			return (*pos).get(); // Dereference iterator to unique_ptr of tuple
		}
		return nullptr;
	}

	swap_chain_data* vulkan::get_surf_swap_tuple_for_surface(const vk::SurfaceKHR& pSurface)
	{
		auto pos = std::find_if(
			std::begin(mSurfSwap), std::end(mSurfSwap),
			[&pSurface](const swap_chain_data_ptr& ptr_to_tpl) {
				return pSurface == ptr_to_tpl->mSurface;
			});
		if (pos != mSurfSwap.end()) {
			return (*pos).get(); // Dereference iterator to unique_ptr of tuple
		}
		return nullptr;
	}

	swap_chain_data* vulkan::get_surf_swap_tuple_for_swap_chain(const vk::SwapchainKHR& pSwapChain)
	{
		auto pos = std::find_if(
			std::begin(mSurfSwap), std::end(mSurfSwap),
			[&pSwapChain](const swap_chain_data_ptr& ptr_to_tpl) {
				return pSwapChain == ptr_to_tpl->mSwapChain;
			});
		if (pos != mSurfSwap.end()) {
			return (*pos).get(); // Dereference iterator to unique_ptr of tuple
		}
		return nullptr;
	}

	std::vector<const char*> vulkan::get_all_required_device_extensions()
	{
		std::vector<const char*> combined;
		combined.assign(std::begin(settings::gRequiredDeviceExtensions), std::end(settings::gRequiredDeviceExtensions));
		combined.insert(std::end(combined), std::begin(sRequiredDeviceExtensions), std::end(sRequiredDeviceExtensions));
		return combined;
	}

	bool vulkan::supports_all_required_extensions(const vk::PhysicalDevice& device)
	{
		bool allExtensionsSupported = true;
		auto allRequiredDeviceExtensions = get_all_required_device_extensions();
		if (allRequiredDeviceExtensions.size() > 0) {
			// Search for each extension requested!
			for (const auto& required : allRequiredDeviceExtensions) {
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

	void vulkan::create_and_assign_logical_device(vk::SurfaceKHR pSurface)
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
				.setQueueFamilyIndex(std::get<0>(familiesWithGraphicsSupport[0])) // Question: Is it okay to just select the first queue family? (Why not the others?)
				.setQueueCount(1u) // The currently available drivers will only allow you to create a small number of queues for each queue family and you don't really need more than one. [1]
				.setPQueuePriorities(&queuePriority);
			// 2. add the present queue:
			queueCreateInfos.emplace_back()
				.setQueueFamilyIndex(std::get<0>(familiesWithPresentSupport[0])) // Question: Is it okay to just select the first queue family? (Why not the others?)
				.setQueueCount(1u) // The currently available drivers will only allow you to create a small number of queues for each queue family and you don't really need more than one. [1]
				.setPQueuePriorities(&queuePriority);
		}
		else {
			// Found a queue which can handle both, graphics and present => add only one instead of two
			queueCreateInfos.emplace_back()
				.setQueueFamilyIndex(std::get<0>(familiesWithGrahicsAndPresentSupport[0])) // Question: Is it okay to just select the first queue family? (Why not the others?)
				.setQueueCount(1u) // The currently available drivers will only allow you to create a small number of queues for each queue family and you don't really need more than one. [1]
				.setPQueuePriorities(&queuePriority);
		}

		// Get the same validation layers as for the instance!
		std::vector<const char*> supportedValidationLayers = assemble_validation_layers();
		
		auto deviceFeatures = vk::PhysicalDeviceFeatures();
		auto allRequiredDeviceExtensions = get_all_required_device_extensions();
		auto deviceCreateInfo = vk::DeviceCreateInfo()
			.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
			.setPQueueCreateInfos(queueCreateInfos.data())
			.setPEnabledFeatures(&deviceFeatures)
			// Whether the device supports these extensions has already been checked during device selection in @ref pick_physical_device
			// TODO: Are these the correct extensions to set here?
			.setEnabledExtensionCount(static_cast<uint32_t>(allRequiredDeviceExtensions.size()))
			.setPpEnabledExtensionNames(allRequiredDeviceExtensions.data())
			.setEnabledLayerCount(static_cast<uint32_t>(supportedValidationLayers.size()))
			.setPpEnabledLayerNames(supportedValidationLayers.data());
		mLogicalDevice = mPhysicalDevice.createDevice(deviceCreateInfo);

		if (familiesWithGrahicsAndPresentSupport.size() == 0) {
			mGraphicsQueue = mLogicalDevice.getQueue(std::get<0>(familiesWithGraphicsSupport[0]), 0u);
			mPresentQueue = mLogicalDevice.getQueue(std::get<0>(familiesWithPresentSupport[0]), 0u);
		}
		else {
			mGraphicsQueue = mPresentQueue = mLogicalDevice.getQueue(std::get<0>(familiesWithGrahicsAndPresentSupport[0]), 0u);
		}
	}

	swap_chain_data vulkan::create_swap_chain(const window* pWindow, const vk::SurfaceKHR& pSurface, const swap_chain_params& pParams)
	{
		auto srfCaps = mPhysicalDevice.getSurfaceCapabilitiesKHR(pSurface);
		auto srfFrmts = mPhysicalDevice.getSurfaceFormatsKHR(pSurface);
		auto presModes = mPhysicalDevice.getSurfacePresentModesKHR(pSurface);

		// Vulkan tells us to match the resolution of the window by setting the width and height in the 
		// currentExtent member. However, some window managers do allow us to differ here and this is 
		// indicated by setting the width and height in currentExtent to a special value: the maximum 
		// value of uint32_t. In that case we'll pick the resolution that best matches the window within 
		// the minImageExtent and maxImageExtent bounds. [2]
		auto extent = srfCaps.currentExtent.width == std::numeric_limits<uint32_t>::max()
			? glm::clamp(generic_glfw::window_extent(*pWindow),
						 glm::uvec2(srfCaps.minImageExtent.width, srfCaps.minImageExtent.height),
						 glm::uvec2(srfCaps.maxImageExtent.width, srfCaps.maxImageExtent.height))
			: glm::uvec2(srfCaps.currentExtent.width, srfCaps.currentExtent.height);

		// Select a presentation mode:
		decltype(presModes)::iterator selPresModeItr = presModes.end();
		if (pParams.mPresentationMode) {
			switch (*pParams.mPresentationMode) {
			case cgb::presentation_mode::immediate:
				selPresModeItr = std::find(std::begin(presModes), std::end(presModes), vk::PresentModeKHR::eImmediate);
				break;
			case cgb::presentation_mode::double_buffering:
				selPresModeItr = std::find(std::begin(presModes), std::end(presModes), vk::PresentModeKHR::eFifoRelaxed);
				break;
			case cgb::presentation_mode::vsync:
				selPresModeItr = std::find(std::begin(presModes), std::end(presModes), vk::PresentModeKHR::eFifo);
				break;
			case cgb::presentation_mode::triple_buffering:
				selPresModeItr = std::find(std::begin(presModes), std::end(presModes), vk::PresentModeKHR::eMailbox);
				break;
			default:
				throw std::runtime_error("should not get here");
			}
		}
		if (selPresModeItr == presModes.end()) {
			LOG_WARNING_EM("No presentation mode specified or desired presentation mode not available => will select any presentation mode");
			selPresModeItr = presModes.begin();
		}

		// select a format:
		auto selSurfaceFormat = vk::SurfaceFormatKHR{
			vk::Format::eB8G8R8A8Unorm,
			vk::ColorSpaceKHR::eSrgbNonlinear
		};
		if (!(srfFrmts.size() == 1 && srfFrmts[0].format == vk::Format::eUndefined)) {
			for (const auto& e : srfFrmts) {
				if (true == pParams.mFramebufferParams.mSrgbFormat) {
					if (is_srgb_format(cgb::image_format(e))) {
						selSurfaceFormat = e;
						break;
					}
				}
				else {
					if (!is_srgb_format(cgb::image_format(e))) {
						selSurfaceFormat = e;
						break;
					}
				}
			}
		}

		// Select the number of images. 
		// TODO: Should this depend on the selected presentation mode?
		auto imageCount = srfCaps.minImageCount + 1u;
		if (srfCaps.maxImageCount > 0) { // A value of 0 for maxImageCount means that there is no limit
			imageCount = glm::min(imageCount, srfCaps.maxImageCount);
		}

		// With all settings gathered, create the swap chain!
		auto createInfo = vk::SwapchainCreateInfoKHR()
			.setSurface(pSurface)
			.setMinImageCount(imageCount)
			.setImageFormat(selSurfaceFormat.format)
			.setImageColorSpace(selSurfaceFormat.colorSpace)
			.setImageExtent(vk::Extent2D(extent.x, extent.y))
			.setImageArrayLayers(1) // The imageArrayLayers specifies the amount of layers each image consists of. This is always 1 unless you are developing a stereoscopic 3D application. [2]
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setPreTransform(srfCaps.currentTransform) // To specify that you do not want any transformation, simply specify the current transformation. [2]
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque) // => no blending with other windows
			.setPresentMode(*selPresModeItr)
			.setClipped(VK_TRUE) // we don't care about the color of pixels that are obscured, for example because another window is in front of them.  [2]
			.setOldSwapchain({}); // TODO: This won't be enought, I'm afraid/pretty sure. => advanced chapter

		// See if we can find a queue family which satisfies both criteria: graphics AND presentation (on the given surface)
		auto allInclFamilies = find_queue_families_for_criteria(vk::QueueFlagBits::eGraphics, pSurface);
		std::vector<uint32_t> queueFamilyIndices;
		if (allInclFamilies.size() != 0) {
			// Found a queue family which supports both!
			// If the graphics queue family and presentation queue family are the same, which will be the case on most hardware, then we should stick to exclusive mode. [2]
			createInfo
				.setImageSharingMode(vk::SharingMode::eExclusive)
				.setQueueFamilyIndexCount(0) // Optional [2]
				.setPQueueFamilyIndices(nullptr); // Optional [2]
		}
		else {
			auto graphicsFamily = find_queue_families_for_criteria(vk::QueueFlagBits::eGraphics, std::nullopt);
			auto presentFamily = find_queue_families_for_criteria(std::nullopt, pSurface);
			assert(graphicsFamily.size() > 0);
			assert(presentFamily.size() > 0);
			queueFamilyIndices.push_back(std::get<0>(graphicsFamily[0]));
			queueFamilyIndices.push_back(std::get<0>(presentFamily[0]));
			// Have to use separate queue families!
			// If the queue families differ, then we'll be using the concurrent mode [2]
			createInfo
				.setImageSharingMode(vk::SharingMode::eConcurrent)
				.setQueueFamilyIndexCount(static_cast<uint32_t>(queueFamilyIndices.size()))
				.setPQueueFamilyIndices(queueFamilyIndices.data());
		}

		// Finally, create the swap chain prepare a struct which stores all relevant data (for further use)
		auto swapChainData = swap_chain_data
		{
			const_cast<window*>(pWindow),
			pSurface,
			mLogicalDevice.createSwapchainKHR(createInfo),
			selSurfaceFormat,
			vk::Extent2D(extent.x, extent.y),
			{}, // std::vector<vk::Image> mSwapChainImages
			{}  // std::vector<vk::ImageView> mSwapChainImageViews
		};
		auto swapChainImages = mLogicalDevice.getSwapchainImagesKHR(swapChainData.mSwapChain);
		// Store the images,
		std::copy(std::begin(swapChainImages), std::end(swapChainImages),
				  std::back_inserter(swapChainData.mSwapChainImages));
		// and create one image view per image
		std::transform(std::begin(swapChainData.mSwapChainImages), std::end(swapChainData.mSwapChainImages),
					   std::back_inserter(swapChainData.mSwapChainImageViews),
					   [&swapChainData, this](const auto& image) {
						   auto viewCreateInfo = vk::ImageViewCreateInfo()
							   .setImage(image)
							   .setViewType(vk::ImageViewType::e2D)
							   .setFormat(swapChainData.mSwapChainImageFormat.mFormat)
							   .setComponents(vk::ComponentMapping() // The components field allows you to swizzle the color channels around. In our case we'll stick to the default mapping. [3]
											  .setR(vk::ComponentSwizzle::eIdentity)
											  .setG(vk::ComponentSwizzle::eIdentity)
											  .setB(vk::ComponentSwizzle::eIdentity)
											  .setA(vk::ComponentSwizzle::eIdentity))
							   .setSubresourceRange(vk::ImageSubresourceRange() // The subresourceRange field describes what the image's purpose is and which part of the image should be accessed. Our images will be used as color targets without any mipmapping levels or multiple layers. [3]
													.setAspectMask(vk::ImageAspectFlagBits::eColor)
													.setBaseMipLevel(0u)
													.setLevelCount(1u)
													.setBaseArrayLayer(0u)
													.setLayerCount(1u));
						   // Note:: If you were working on a stereographic 3D application, then you would create a swap chain with multiple layers. You could then create multiple image views for each image representing the views for the left and right eyes by accessing different layers. [3]
						   auto imageView = mLogicalDevice.createImageView(viewCreateInfo);
						   return imageView;
					   });

		return swapChainData;
	}

	vk::RenderPass vulkan::create_render_pass(image_format pImageFormat)
	{
		auto colorAttachment = vk::AttachmentDescription()
			.setFormat(pImageFormat.mFormat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear) // what to do with the data in the attachment before rendering and after rendering [5]
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined) // we don't care what previous layout the image was in. The caveat of this special value is that the contents of the image are not guaranteed to be preserved, but that doesn't matter since we're going to clear it anyway. [5]
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR); //  Images to be presented in the swap chain [5]

		// Attachment references for subpasses
		auto colorAttachmentRef = vk::AttachmentReference()
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal); // We intend to use the attachment to function as a color buffer => this layout will give us the best performance [5]

		auto subpassDesc = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1u)
			.setPColorAttachments(&colorAttachmentRef);
		// The following other types of attachments can be referenced by a subpass [5]:
		// - pInputAttachments: Attachments that are read from a shader
		// - pResolveAttachments: Attachments used for multisampling color attachments
		// - pDepthStencilAttachment : Attachments for depth and stencil data
		// - pPreserveAttachments : Attachments that are not used by this subpass, but for which the data must be preserved

		auto subpassDependency = vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL) // The special value VK_SUBPASS_EXTERNAL refers to the implicit subpass before or after the render pass depending on whether it is specified in srcSubpass or dstSubpass. [8]
			.setDstSubpass(0) // TODO: What to do when there are more subpasses?
			// The dstSubpass must always be higher than srcSubpass to prevent cycles in the dependency graph. [8]
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlags())
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

		// Create the render pass
		auto renderPassInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(1u)
			.setPAttachments(&colorAttachment)
			.setSubpassCount(1u)
			.setPSubpasses(&subpassDesc)
			.setDependencyCount(1u)
			.setPDependencies(&subpassDependency);
		return mLogicalDevice.createRenderPass(renderPassInfo); // TODO: use this
	}

	pipeline vulkan::create_graphics_pipeline_for_window(const std::vector<std::tuple<shader_type, shader_handle*>>& pShaderInfos, const window* pWindow, const vk::VertexInputBindingDescription& pBindingDesc, const std::array<vk::VertexInputAttributeDescription, 2>& pAttributeDesc)
	{
		auto data = get_surf_swap_tuple_for_window(pWindow);
		assert(data);
		return create_graphics_pipeline_for_swap_chain(pShaderInfos, *data, pBindingDesc, pAttributeDesc);
	}

	pipeline vulkan::create_graphics_pipeline_for_swap_chain(const std::vector<std::tuple<shader_type, shader_handle*>>& pShaderInfos, const swap_chain_data& pSwapChainData, const vk::VertexInputBindingDescription& pBindingDesc, const std::array<vk::VertexInputAttributeDescription, 2>& pAttributeDesc)
	{
		// GATHER ALL THE SHADER INFORMATION
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::transform(std::begin(pShaderInfos), std::end(pShaderInfos),
					   std::back_inserter(shaderStages),
					   [](const auto& tpl) {
						   return vk::PipelineShaderStageCreateInfo()
							   .setStage(convert(std::get<shader_type>(tpl)))
							   .setModule(std::get<shader_handle*>(tpl)->mShaderModule)
							   .setPName("main"); // TODO: support different entry points?!
					   });
		
		// DESCRIBE THE VERTEX INPUT
		auto vertexInputinfo = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(1u)
			.setPVertexBindingDescriptions(&pBindingDesc)
			.setVertexAttributeDescriptionCount(static_cast<uint32_t>(pAttributeDesc.size()))
			.setPVertexAttributeDescriptions(pAttributeDesc.data());

		// HOW TO INTERPRET THE VERTEX INPUT
		auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(VK_FALSE);

		// VIEWPORT AND SCISSORS
		auto viewport = vk::Viewport()
			.setX(0.0f)
			.setY(0.0f)
			// Remember that the size of the swap chain and its images may differ from the WIDTH and HEIGHT of the window.The swap chain images will be used as framebuffers later on, so we should stick to their size. [4]
			.setWidth(static_cast<float>(pSwapChainData.mSwapChainExtent.width))
			.setHeight(static_cast<float>(pSwapChainData.mSwapChainExtent.height))
			// These values must be within the [0.0f, 1.0f] range, but minDepth may be higher than maxDepth. If you aren't doing anything special, then you should stick to the standard values of 0.0f and 1.0f. [4]
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);

		auto scissor = vk::Rect2D()
			.setOffset(vk::Offset2D(0, 0))
			.setExtent(pSwapChainData.mSwapChainExtent);

		auto viewportInfo = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1u)
			.setPViewports(&viewport)
			.setScissorCount(1u)
			.setPScissors(&scissor);

		// RASTERIZATION STATE
		auto rasterizer = vk::PipelineRasterizationStateCreateInfo()
			.setDepthClampEnable(VK_FALSE) // If depthClampEnable is set to VK_TRUE, then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them. [4]
			.setRasterizerDiscardEnable(VK_FALSE) // If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes through the rasterizer stage. [4]
			.setPolygonMode(vk::PolygonMode::eFill) // fill the polygon || draw wireframe || draw points
			.setLineWidth(1.0f) // Even if we're not using wireframe mode, we still have to set it, otherwise we'll get a Vulkan error
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eClockwise)
			.setDepthBiasEnable(VK_FALSE) // The rasterizer can alter the depth values by adding a constant value or biasing them based on a fragment's slope. This is sometimes used for shadow mapping [4]
			.setDepthBiasConstantFactor(0.0f) // Optional
			.setDepthBiasClamp(0.0f) // Optional
			.setDepthBiasSlopeFactor(0.0f); // Optional

		// MULTISAMPLING
		auto multisampling = vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(VK_FALSE) // disable
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setMinSampleShading(1.0f) // Optional
			.setPSampleMask(nullptr) // Optional
			.setAlphaToCoverageEnable(VK_FALSE) // Optional
			.setAlphaToOneEnable(VK_FALSE); // Optional

		// DEPTH AND STENCIL TESTING
		auto depthStencil = vk::PipelineDepthStencilStateCreateInfo();

		// COLOR BLENDING
		auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(VK_FALSE) // If blendEnable is set to VK_FALSE, then the new color from the fragment shader is passed through unmodified. [4]
			.setSrcColorBlendFactor(vk::BlendFactor::eOne) // Optional
			.setDstColorBlendFactor(vk::BlendFactor::eZero) // Optional
			.setColorBlendOp(vk::BlendOp::eAdd) // Optional
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne) // Optional
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero) // Optional
			.setAlphaBlendOp(vk::BlendOp::eAdd); // Optional
		auto colorBlendingInfo = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(VK_FALSE) // If you want to use the second method of blending (bitwise combination), then you should set logicOpEnable to VK_TRUE. The bitwise operation can then be specified in the logicOp field. [4]
			.setLogicOp(vk::LogicOp::eCopy) // Optional
			.setAttachmentCount(1u)
			.setPAttachments(&colorBlendAttachment)
			.setBlendConstants({ {0.0f, 0.0f, 0.0f, 0.0f} }); // Optional

		// DYNAMIC STATE
		// A limited amount of the state that we've specified in the previous structs can actually be changed without recreating the pipeline. 
		// Examples are the size of the viewport, line width and blend constants. If you want to do that, then you'll have to fill in a vk::PipelineDynamicStateCreateInfo structure. [4]
		auto dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eLineWidth };
		auto dynamicStateInfo = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
			.setPDynamicStates(std::begin(dynamicStates));

		// PIPELINE LAYOUT
		// These uniform values (Anm.: passed to shaders) need to be specified during pipeline creation by creating a VkPipelineLayout object. [4]
		auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(0u)
			.setPSetLayouts(nullptr)
			.setPushConstantRangeCount(0u)
			.setPPushConstantRanges(nullptr);
		auto pipelineLayout = mLogicalDevice.createPipelineLayout(pipelineLayoutInfo); 


		// CREATE RENDER PASS (for sure, this is the wrong place to do so => refactor!)
		auto renderPass = create_render_pass(pSwapChainData.mSwapChainImageFormat);


		// PIPELINE CREATION
		auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
			.setStageCount(static_cast<uint32_t>(shaderStages.size()))
			.setPStages(shaderStages.data())
			.setPVertexInputState(&vertexInputinfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportInfo)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisampling)
			.setPDepthStencilState(nullptr) // Optional
			.setPColorBlendState(&colorBlendingInfo)
			.setPDynamicState(nullptr) // Optional
			.setLayout(pipelineLayout)
			.setRenderPass(renderPass)
			.setSubpass(0u)
			.setBasePipelineHandle(nullptr) // Optional
			.setBasePipelineIndex(-1); // Optional

		// Create the pipeline, return it and also store the render pass and the pipeline layout in the struct!
		return pipeline(
			renderPass,
			pipelineLayout, 
			mLogicalDevice.createGraphicsPipeline(
				nullptr, // references an optional VkPipelineCache object. A pipeline cache can be used to store and reuse data relevant to pipeline creation across multiple calls to vkCreateGraphicsPipelines and even across program executions [5]
				pipelineInfo));
	}

	std::vector<framebuffer> vulkan::create_framebuffers(const vk::RenderPass& renderPass, const window* pWindow)
	{
		auto data = get_surf_swap_tuple_for_window(pWindow);
		assert(data);
		return create_framebuffers(renderPass, *data);
	}

	std::vector<framebuffer> vulkan::create_framebuffers(const vk::RenderPass& renderPass, const swap_chain_data& pSwapChainData)
	{
		std::vector<framebuffer> framebuffers;
		for (auto& imageView : pSwapChainData.mSwapChainImageViews) {
			auto framebufferInfo = vk::FramebufferCreateInfo()
				.setRenderPass(renderPass)
				.setAttachmentCount(1u)
				.setPAttachments(&imageView)
				.setWidth(pSwapChainData.mSwapChainExtent.width)
				.setHeight(pSwapChainData.mSwapChainExtent.height)
				.setLayers(1u); // number of layers in image arrays [6]

			framebuffers.push_back(framebuffer{ mLogicalDevice.createFramebuffer( framebufferInfo ) });
		}
		return framebuffers;
	}

	command_pool vulkan::create_command_pool()
	{
		auto familiesWithGraphicsSupport = find_queue_families_for_criteria(vk::QueueFlagBits::eGraphics, std::nullopt);
		auto commandPoolInfo = vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(std::get<uint32_t>(familiesWithGraphicsSupport[0]))
			.setFlags(vk::CommandPoolCreateFlags()); // Optional
		// Possible values for the flags [7]
		//  - VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
		//  - VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together

		return command_pool{ mLogicalDevice.createCommandPool(commandPoolInfo) };
	}

	std::vector<command_buffer> vulkan::create_command_buffers(uint32_t pCount, const command_pool& pCommandPool)
	{
		auto bufferAllocInfo = vk::CommandBufferAllocateInfo()
			.setCommandPool(pCommandPool.mCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary) // TODO: make configurable?!
			.setCommandBufferCount(pCount);

		std::vector<command_buffer> buffers;
		auto tmp = mLogicalDevice.allocateCommandBuffers(bufferAllocInfo);
		std::transform(std::begin(tmp), std::end(tmp),
					   std::back_inserter(buffers),
					   [](const auto& vkCb) { return command_buffer{ vkCb }; });
		return buffers;
	}

	uint32_t vulkan::find_memory_type_index(uint32_t pMemoryTypeBits, vk::MemoryPropertyFlags pMemoryProperties)
	{
		// The VkPhysicalDeviceMemoryProperties structure has two arrays memoryTypes and memoryHeaps. 
		// Memory heaps are distinct memory resources like dedicated VRAM and swap space in RAM for 
		// when VRAM runs out. The different types of memory exist within these heaps. Right now we'll 
		// only concern ourselves with the type of memory and not the heap it comes from, but you can 
		// imagine that this can affect performance.
		auto memProperties = mPhysicalDevice.getMemoryProperties();
		for (auto i = 0u; i < memProperties.memoryTypeCount; ++i) {
			if ((pMemoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & pMemoryProperties) == pMemoryProperties) {
				return i;
			}
		}
		throw std::runtime_error("failed to find suitable memory type!");
	}

	// REFERENCES:
	// [1] Vulkan Tutorial, Logical device and queues, https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues
	// [2] Vulkan Tutorial, Swap chain, https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
	// [3] Vulkan Tutorial, Image views, https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Image_views
	// [4] Vulkan Tutorial, Fixed functions, https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
	// [5] Vulkan Tutorial, Render passes, https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Render_passes
	// [6] Vulkan Tutorial, Framebuffers, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Framebuffers
	// [7] Vulkan Tutorial, Command Buffers, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Command_buffers
	// [8] Vulkan Tutorial, Rendering and presentation, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
	// [9] Vulkan Tutorial, Vertex buffers, https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
}
