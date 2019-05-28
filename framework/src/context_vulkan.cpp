#include "context_vulkan.h"
#include <set>

namespace cgb
{
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
					LOG_WARNING(fmt::format("Validation layer '{}' is not supported by this Vulkan instance and will not be activated.", name));
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

		// The window surface needs to be created right after the instance creation 
		// and before physical device selection, because it can actually influence 
		// the physical device selection.

		mContextState = cgb::context_state::halfway_initialized;

		// NOTE: Vulkan-init is not finished yet!
		// Initialization will continue after the first window (and it's surface) have been created.
		// Only after the first window's surface has been created, the vulkan context can complete
		//   initialization and enter the context state of fully_initialized.
		//
		// Attention: May not use the `add_event_handler`-method here, because it would internally
		//   make use of `cgb::context()` which would refer to this static instance, which has not 
		//   yet finished initialization => would deadlock; Instead, modify data structure directly. 
		//   This constructor is the only exception, in all other cases, it's safe to use `add_event_handler`
		//   
		mEventHandlers.emplace_back([]() -> bool {
			LOG_INFO("Running vulkan fully_init event handler");

			// Just get any window:
			auto* window = context().find_window([](cgb::window* w) { 
				return w->handle().has_value() && static_cast<bool>(w->surface());
			});

			// Do we have a window with a handle?
			if (nullptr == window) { 
				return false; // Nope => not done
			}

			// We need a SURFACE to create the logical device => do it after the first window has been created
			auto& surface = window->surface();

			// Select the best suitable physical device which supports all requested extensions
			context().pick_physical_device();

			context().create_and_assign_logical_device(surface);

		}, cgb::context_state::halfway_initialized);

		// We're still not done yet with our initial setup efforts if we need ImGui,
		// but we're going to set that up at an even later point -> when we have the 
		// context fully initiylized:
		if (!settings::gDisableImGui) {
			// Init and wire-in IMGUI
			//
			// Attention: May not use the `add_event_handler`-method here, because it would internally
			//   make use of `cgb::context()` which would refer to this static instance, which has not 
			//   yet finished initialization => would deadlock; Instead, modify data structure directly. 
			//   This constructor is the only exception, in all other cases, it's safe to use `add_event_handler`
			//   
			mEventHandlers.emplace_back([]() -> bool {
				LOG_INFO("Running IMGUI setup event handler");

				// Just get any window:
				auto* window = context().find_window([](cgb::window* w) { 
					return w->handle().has_value();
				});

				// Do we have a window with a handle?
				if (nullptr == window) { 
					return false; // Nope => not done
				}

				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
				//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

				// Setup Dear ImGui style
				ImGui::StyleColorsDark();
				//ImGui::StyleColorsClassic();

				// Setup Platform/Renderer bindings
				ImGui_ImplGlfw_InitForVulkan(window->handle()->mHandle, true); // TODO: Don't install callbacks

				//struct ImGui_ImplVulkan_InitInfo
				//{
				//	VkInstance          Instance;
				//	VkPhysicalDevice    PhysicalDevice;
				//	VkDevice            Device;
				//	uint32_t            QueueFamily;
				//	VkQueue             Queue;
				//	VkPipelineCache     PipelineCache;
				//	VkDescriptorPool    DescriptorPool;
				//	uint32_t            MinImageCount;          // >= 2
				//	uint32_t            ImageCount;             // >= MinImageCount
				//	const VkAllocationCallbacks* Allocator;
				//	void                (*CheckVkResultFn)(VkResult err);
				//};

				ImGui_ImplVulkan_InitInfo init_info = {};
				init_info.Instance = context().vulkan_instance();
				init_info.PhysicalDevice = context().physical_device();
				init_info.Device = context().logical_device();;
				init_info.QueueFamily = context().graphics_queue_index();
				init_info.Queue = context().graphics_queue();
				init_info.PipelineCache = VK_NULL_HANDLE;
				init_info.DescriptorPool = context().get_descriptor_pool().mDescriptorPool;
				init_info.Allocator = VK_NULL_HANDLE;
				//init_info.MinImageCount = sActualMaxFramesInFlight; // <---- TODO
				//init_info.ImageCount = sActualMaxFramesInFlight; // <---- TODO
				init_info.CheckVkResultFn = check_vk_result;

				// TODO: Hmm, or do we have to do this per swap chain? 
			}, cgb::context_state::fully_initialized);
		}
	}

	vulkan::~vulkan()
	{
		mContextState = cgb::context_state::about_to_finalize;
		while (work_off_event_handlers() > 0u);

		// Destroy all descriptor pools before the queues and the device is destroyed
		mDescriptorPools.clear();

		// Destroy all command pools before the queues and the device is destroyed
		mCommandPools.clear();

		//// Destroy all:
		////  - swap chains,
		////  - surfaces,
		////  - and windows
		//for (auto& ptrToSwapChainData : mSurfSwap) {
		//	// Unlike images, the image views were explicitly created by us, so we need to add a similar loop to destroy them again at the end of the program [3]
		//	for (auto& imageView : ptrToSwapChainData->mSwapChainImageViews) {
		//		mLogicalDevice.destroyImageView(imageView);
		//	}
		//	mLogicalDevice.destroySwapchainKHR(ptrToSwapChainData->mSwapChain);
		//	mInstance.destroySurfaceKHR(ptrToSwapChainData->mSurface);
		//	generic_glfw::close_window(*ptrToSwapChainData->mWindow);
		//}
		//mSurfSwap.clear();

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
	
		mContextState = cgb::context_state::has_finalized;
		while (work_off_event_handlers() > 0u);
	}

	void vulkan::check_vk_result(VkResult err)
	{
		createResultValue(static_cast<vk::Result>(err), context().vulkan_instance(), "check_vk_result");
	}

	void vulkan::begin_composition()
	{ 
		dispatch_to_main_thread([]() {
			context().mContextState = cgb::context_state::composition_beginning;
			while (context().work_off_event_handlers() > 0u);
		});
	}

	void vulkan::end_composition()
	{
		dispatch_to_main_thread([]() {
			context().mContextState = cgb::context_state::composition_ending;
			while (context().work_off_event_handlers() > 0u);
			context().mLogicalDevice.waitIdle();
		});
	}

	void vulkan::begin_frame()
	{
		dispatch_to_main_thread([]() {
			context().mContextState = cgb::context_state::frame_begun;
			while (context().work_off_event_handlers() > 0u);
		});
		//mFrameCounter += 1;
	
		// Wait for the prev-prev frame (fence-ping-pong)
		// TODO: We should only wait for fences if some were submitted 
		//       ...during the last RENDER-call!!!
		
		// TODO: auskommentiert während StSt-Meeting

		//auto& fence = fence_current_frame();
		//mLogicalDevice.waitForFences(1u, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		//mLogicalDevice.resetFences(1u, &fence);
	}

	void vulkan::update_stage_done()
	{
		dispatch_to_main_thread([]() {
			context().mContextState = cgb::context_state::frame_updates_done;
			while (context().work_off_event_handlers() > 0u);
		});
	}

	void vulkan::end_frame()
	{
		dispatch_to_main_thread([]() {
			context().mContextState = cgb::context_state::frame_ended;
			while (context().work_off_event_handlers() > 0u);
		});
	}

	void vulkan::draw_triangle(const pipeline& pPipeline, const command_buffer& pCommandBuffer)
	{
		pCommandBuffer.mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pPipeline.mPipeline);
		pCommandBuffer.mCommandBuffer.draw(3u, 1u, 0u, 0u);
	}

	void vulkan::draw_vertices(const pipeline& pPipeline, const command_buffer& pCommandBuffer, const vertex_buffer& pVertexBuffer)
	{
		pCommandBuffer.mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pPipeline.mPipeline);
		pCommandBuffer.mCommandBuffer.bindVertexBuffers(0u, { pVertexBuffer.mBuffer }, { 0 });
		pCommandBuffer.mCommandBuffer.draw(pVertexBuffer.mVertexCount, 1u, 0u, 0u);
	}

	void vulkan::draw_indexed(const pipeline& pPipeline, const command_buffer& pCommandBuffer, const vertex_buffer& pVertexBuffer, const index_buffer& pIndexBuffer)
	{
		pCommandBuffer.mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pPipeline.mPipeline);
		pCommandBuffer.mCommandBuffer.bindVertexBuffers(0u, { pVertexBuffer.mBuffer }, { 0 });
		pCommandBuffer.mCommandBuffer.bindIndexBuffer(pIndexBuffer.mBuffer, 0u, pIndexBuffer.mIndexType);
		pCommandBuffer.mCommandBuffer.drawIndexed(pIndexBuffer.mIndexCount, 1u, 0u, 0u, 0u);
	}

	window* vulkan::create_window(const std::string& pTitle)
	{
		assert(are_we_on_the_main_thread());
		while (work_off_event_handlers() > 0u);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		auto* wnd = generic_glfw::prepare_window();

		// Wait for the window to receive a valid handle before creating its surface
		context().add_event_handler(context_state::halfway_initialized | context_state::anytime_after_init_before_finalize, [wnd]() -> bool {
			LOG_INFO("Running window surface creator event handler");
			
			// Make sure it is the right window
			auto* window = context().find_window([wnd](cgb::window* w) { 
				return w == wnd && w->handle().has_value();
			});

			if (nullptr == window) { // not yet
				return false;
			}

			VkSurfaceKHR surface;
			if (VK_SUCCESS != glfwCreateWindowSurface(context().vulkan_instance(), wnd->handle()->mHandle, nullptr, &surface)) {
				throw std::runtime_error(fmt::format("Failed to create surface for window '{}'!", wnd->title()));
			}
			window->mSurface = surface;
			return true;
		});

		// Continue with swap chain creation after the context has completely initialized
		//   and the window's handle and surface have been created
		context().add_event_handler(context_state::anytime_after_init_before_finalize, [wnd]() -> bool {
			LOG_INFO("Running swap chain creator event handler");

			// Make sure it is the right window
			auto* window = context().find_window([wnd](cgb::window* w) { 
				return w == wnd && w->handle().has_value() && static_cast<bool>(w->surface());
			});

			if (nullptr == window) {
				return false;
			}
			// Okay, the window has a surface and vulkan has initialized. 
			// Let's create more stuff for this window!
			context().create_swap_chain_for_window(wnd);
			return true;
		});

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

	VKAPI_ATTR VkBool32 VKAPI_CALL vulkan::vk_debug_callback(
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
			LOG_ERROR__(fmt::format("Vk-callback with Id[{}|{}] and Message[{}]",
				pCallbackData->messageIdNumber, 
				pCallbackData->pMessageIdName,
				pCallbackData->pMessage));
		}
		else if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			assert(pCallbackData);
			LOG_WARNING__(fmt::format("Vk-callback with Id[{}|{}] and Message[{}]",
				pCallbackData->messageIdNumber,
				pCallbackData->pMessageIdName,
				pCallbackData->pMessage));
		}
		else if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			assert(pCallbackData);
			LOG_INFO__(fmt::format("Vk-callback with Id[{}|{}] and Message[{}]",
				pCallbackData->messageIdNumber,
				pCallbackData->pMessageIdName,
				pCallbackData->pMessage));
		}
		else if (pMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
			assert(pCallbackData);
			LOG_VERBOSE__(fmt::format("Vk-callback with Id[{}|{}] and Message[{}]",
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

	std::vector<const char*> vulkan::get_all_required_device_extensions()
	{
		std::vector<const char*> combined;
		combined.assign(std::begin(settings::gRequiredDeviceExtensions), std::end(settings::gRequiredDeviceExtensions));
		combined.insert(std::end(combined), std::begin(sRequiredDeviceExtensions), std::end(sRequiredDeviceExtensions));
		return combined;
	}

	bool vulkan::supports_shading_rate_image(const vk::PhysicalDevice& device)
	{
		vk::PhysicalDeviceFeatures2 supportedExtFeatures;
		auto shadingRateImageFeatureNV = vk::PhysicalDeviceShadingRateImageFeaturesNV();
		supportedExtFeatures.pNext = &shadingRateImageFeatureNV;
		device.getFeatures2(&supportedExtFeatures);
		return shadingRateImageFeatureNV.shadingRateImage && shadingRateImageFeatureNV.shadingRateCoarseSampleOrder && supportedExtFeatures.features.shaderStorageImageExtendedFormats;
	}

	bool vulkan::shading_rate_image_extension_requested()
	{
		auto allRequiredDeviceExtensions = get_all_required_device_extensions();
		return std::find(std::begin(allRequiredDeviceExtensions), std::end(allRequiredDeviceExtensions), VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME) != std::end(allRequiredDeviceExtensions);
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

		auto shadingRateImageExtensionRequired = shading_rate_image_extension_requested();
		allExtensionsSupported = allExtensionsSupported && (!shadingRateImageExtensionRequired || shadingRateImageExtensionRequired && supports_shading_rate_image(device));

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
			auto supportedFeatures = device.getFeatures();
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

			// Check if anisotropy is supported
			if (!supportedFeatures.samplerAnisotropy) {
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

	std::vector<std::tuple<uint32_t, vk::QueueFamilyProperties>> vulkan::find_queue_families_for_criteria(
		vk::QueueFlags pRequiredFlags, 
		vk::QueueFlags pForbiddenFlags, 
		std::optional<vk::SurfaceKHR> pSurface)
	{
		assert(mPhysicalDevice);
		// All queue families:
		auto queueFamilies = physical_device().getQueueFamilyProperties();
		std::vector<std::tuple<uint32_t, vk::QueueFamilyProperties>> indexedQueueFamilies;
		std::transform(std::begin(queueFamilies), std::end(queueFamilies),
					   std::back_inserter(indexedQueueFamilies),
					   [index = uint32_t(0)](const decltype(queueFamilies)::value_type& input) mutable {
						   auto tpl = std::make_tuple(index, input);
						   index += 1;
						   return tpl;
					   });
		// Subset to which the criteria applies:
		std::vector<std::tuple<uint32_t, vk::QueueFamilyProperties>> selection;
		// Select the subset
		std::copy_if(std::begin(indexedQueueFamilies), std::end(indexedQueueFamilies),
					 std::back_inserter(selection),
					 [pRequiredFlags, pForbiddenFlags, pSurface, this](const std::tuple<uint32_t, decltype(queueFamilies)::value_type>& tpl) {
						 bool requirementsMet = true;
						 if (pRequiredFlags) {
							 requirementsMet = requirementsMet && ((std::get<1>(tpl).queueFlags & pRequiredFlags) == pRequiredFlags);
						 }
						 if (pForbiddenFlags) {
							 requirementsMet = requirementsMet && ((std::get<1>(tpl).queueFlags & pForbiddenFlags) != pForbiddenFlags);
						 }
						 if (pSurface) {
							 requirementsMet = requirementsMet && (mPhysicalDevice.getSurfaceSupportKHR(std::get<0>(tpl), *pSurface));
						 }
						 return requirementsMet;
					 });
		return selection;
	}

	std::vector<std::tuple<uint32_t, vk::QueueFamilyProperties>> vulkan::find_best_queue_family_for(vk::QueueFlags pRequiredFlags, device_queue_selection_strategy pSelectionStrategy, std::optional<vk::SurfaceKHR> pSurface)
	{
		static std::array queueTypes = {
			vk::QueueFlagBits::eGraphics,
			vk::QueueFlagBits::eCompute,
			vk::QueueFlagBits::eTransfer,
		};
		// sparse binding and protected bits are ignored, for now

		decltype(std::declval<vulkan>().find_queue_families_for_criteria(vk::QueueFlags(), vk::QueueFlags(), std::nullopt)) selection;
		
		vk::QueueFlags forbiddenFlags = {};
		for (auto f : queueTypes) {
			forbiddenFlags |= f;
		}
		forbiddenFlags &= ~pRequiredFlags;

		int32_t loosenIndex = 0;
		
		switch (pSelectionStrategy) {
		case cgb::device_queue_selection_strategy::prefer_separate_queues:
			while (loosenIndex <= queueTypes.size()) { // might result in returning an empty selection
				selection = find_queue_families_for_criteria(pRequiredFlags, forbiddenFlags, pSurface);
				if (selection.size() > 0 || loosenIndex == queueTypes.size()) {
					break;
				}
				forbiddenFlags = forbiddenFlags & ~queueTypes[loosenIndex++]; // gradually loosen restrictions
			};
			break;
		case cgb::device_queue_selection_strategy::prefer_everything_on_single_queue:
			// Nothing is forbidden
			selection = find_queue_families_for_criteria(pRequiredFlags, vk::QueueFlags(), pSurface);
			break;
		}

		return selection;
	}

	void vulkan::create_and_assign_logical_device(vk::SurfaceKHR pSurface)
	{
		assert(mPhysicalDevice);

		// Determine which queue families we have, i.e. what the different queue families support and what they don't
		auto* presentQueue		= device_queue::prepare(vk::QueueFlagBits::eGraphics,		settings::gQueueSelectionPreference, pSurface);
		auto* graphicsQueue		= device_queue::prepare(vk::QueueFlagBits::eGraphics,		settings::gPreferSameQueueForGraphicsAndPresent 
																							  ? device_queue_selection_strategy::prefer_everything_on_single_queue
																							  : settings::gQueueSelectionPreference, std::nullopt);
		auto* computeQueue		= device_queue::prepare(vk::QueueFlagBits::eCompute,		settings::gQueueSelectionPreference, std::nullopt);
		auto* transferQueue		= device_queue::prepare(vk::QueueFlagBits::eTransfer,		settings::gQueueSelectionPreference, std::nullopt);

		// Defer pipeline creation to enable the user to request more queues

		mEventHandlers.emplace_back([presentQueue, graphicsQueue, computeQueue, transferQueue]() -> bool {
			LOG_INFO("Running vulkan create_and_assign_logical_device event handler");

			// Get the same validation layers as for the instance!
			std::vector<const char*> supportedValidationLayers = assemble_validation_layers();
		
			// Always prepare the shading rate image features descriptor, but only use it if the extension has been requested
			auto shadingRateImageFeatureNV = vk::PhysicalDeviceShadingRateImageFeaturesNV()
				.setShadingRateImage(VK_TRUE)
				.setShadingRateCoarseSampleOrder(VK_TRUE);
			auto activateShadingRateImage = shading_rate_image_extension_requested() && supports_shading_rate_image(context().physical_device());

			// Gather all the queue create infos:
			std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
			std::vector<std::vector<float>> queuePriorities;
			for (const auto& pq : device_queue::sPreparedQueues) {
				bool handled = false;
				for (auto i = 0; i < queueCreateInfos.size(); ++i) {
					if (queueCreateInfos[i].queueFamilyIndex == pq.family_index()) {
						// found, i.e. increase count
						queueCreateInfos[i].queueCount += 1;
						queuePriorities[i].push_back(pq.mPriority);
						handled = true; 
						break; // done
					}
				}
				if (!handled) { // => must be a new entry
					queueCreateInfos.emplace_back()
						.setQueueFamilyIndex(pq.family_index())
						.setQueueCount(1u);
					queuePriorities.push_back({ pq.mPriority });
				}
			}
			// Iterate over all vk::DeviceQueueCreateInfo entries and set the queue priorities pointers properly
			for (auto i = 0; i < queueCreateInfos.size(); ++i) {
				queueCreateInfos[i].setPQueuePriorities(queuePriorities[i].data());
			}

			// Enable certain device features:
			auto deviceFeatures = vk::PhysicalDeviceFeatures2()
				.setFeatures(vk::PhysicalDeviceFeatures()
					.setSamplerAnisotropy(VK_TRUE)
					.setVertexPipelineStoresAndAtomics(VK_TRUE)
					.setShaderStorageImageExtendedFormats(VK_TRUE))
				.setPNext(activateShadingRateImage ? &shadingRateImageFeatureNV : nullptr);

			auto allRequiredDeviceExtensions = get_all_required_device_extensions();
			auto deviceCreateInfo = vk::DeviceCreateInfo()
				.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
				.setPQueueCreateInfos(queueCreateInfos.data())
				.setPNext(&deviceFeatures) // instead of :setPEnabledFeatures(&deviceFeatures) because we are using vk::PhysicalDeviceFeatures2
				// Whether the device supports these extensions has already been checked during device selection in @ref pick_physical_device
				// TODO: Are these the correct extensions to set here?
				.setEnabledExtensionCount(static_cast<uint32_t>(allRequiredDeviceExtensions.size()))
				.setPpEnabledExtensionNames(allRequiredDeviceExtensions.data())
				.setEnabledLayerCount(static_cast<uint32_t>(supportedValidationLayers.size()))
				.setPpEnabledLayerNames(supportedValidationLayers.data());
			context().mLogicalDevice = context().physical_device().createDevice(deviceCreateInfo);
			// Create a dynamic dispatch loader for extensions
			context().mDynamicDispatch = vk::DispatchLoaderDynamic(context().vulkan_instance(), context().logical_device());

			// Create the queues which have been prepared in the beginning of this method:
			context().mPresentQueue		= device_queue::create(*presentQueue);
			context().mGraphicsQueue	= device_queue::create(*graphicsQueue);
			context().mComputeQueue		= device_queue::create(*computeQueue);
			context().mTransferQueue	= device_queue::create(*transferQueue);

			// Uniqueify queue family indices:
			std::set<uint32_t> uniqueFamilyIndices;
			for (auto q : device_queue::sPreparedQueues) {
				uniqueFamilyIndices.insert(q.family_index());
			}
			// Put into contiguous memory
			for (auto fi : uniqueFamilyIndices) {
				context().mAllUsedQueueFamilyIndices.push_back(fi);
			}

			context().mContextState = cgb::context_state::fully_initialized;

			return true; // This is just always true
		}, cgb::context_state::halfway_initialized);
	}

	void vulkan::create_swap_chain_for_window(window* pWindow)
	{
		auto srfCaps = mPhysicalDevice.getSurfaceCapabilitiesKHR(pWindow->surface());

		// Vulkan tells us to match the resolution of the window by setting the width and height in the 
		// currentExtent member. However, some window managers do allow us to differ here and this is 
		// indicated by setting the width and height in currentExtent to a special value: the maximum 
		// value of uint32_t. In that case we'll pick the resolution that best matches the window within 
		// the minImageExtent and maxImageExtent bounds. [2]
		auto extent = srfCaps.currentExtent.width == std::numeric_limits<uint32_t>::max()
			? glm::clamp(pWindow->resolution(),
						 glm::uvec2(srfCaps.minImageExtent.width, srfCaps.minImageExtent.height),
						 glm::uvec2(srfCaps.maxImageExtent.width, srfCaps.maxImageExtent.height))
			: glm::uvec2(srfCaps.currentExtent.width, srfCaps.currentExtent.height);

		auto surfaceFormat = pWindow->get_config_surface_format(pWindow->surface());

		// With all settings gathered, create the swap chain!
		auto createInfo = vk::SwapchainCreateInfoKHR()
			.setSurface(pWindow->surface())
			.setMinImageCount(pWindow->get_config_number_of_presentable_images())
			.setImageFormat(surfaceFormat.format)
			.setImageColorSpace(surfaceFormat.colorSpace)
			.setImageExtent(vk::Extent2D(extent.x, extent.y))
			.setImageArrayLayers(1) // The imageArrayLayers specifies the amount of layers each image consists of. This is always 1 unless you are developing a stereoscopic 3D application. [2]
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst)
			.setPreTransform(srfCaps.currentTransform) // To specify that you do not want any transformation, simply specify the current transformation. [2]
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque) // => no blending with other windows
			.setPresentMode(pWindow->get_config_presentation_mode(pWindow->surface()))
			.setClipped(VK_TRUE) // we don't care about the color of pixels that are obscured, for example because another window is in front of them.  [2]
			.setOldSwapchain({}); // TODO: This won't be enought, I'm afraid/pretty sure. => advanced chapter

		auto nope = vk::QueueFlags();

		// See if we can find a queue family which satisfies both criteria: graphics AND presentation (on the given surface)
		auto allInclFamilies = find_queue_families_for_criteria(vk::QueueFlagBits::eGraphics, nope, pWindow->surface());
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
			auto graphicsFamily = find_queue_families_for_criteria(vk::QueueFlagBits::eGraphics, nope, std::nullopt);
			auto presentFamily = find_queue_families_for_criteria(nope, nope, pWindow->surface());
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
		pWindow->mSwapChain = logical_device().createSwapchainKHR(createInfo);
		pWindow->mSwapChainImageFormat = surfaceFormat;
		pWindow->mSwapChainExtent = vk::Extent2D(extent.x, extent.y);
		pWindow->mSwapChainImages = {}; // Soon...
		pWindow->mSwapChainImageViews = {}; // Soon...
		pWindow->mCurrentFrame = 0; // Start af frame 0

		auto swapChainImages = logical_device().getSwapchainImagesKHR(pWindow->swap_chain());
		assert(swapChainImages.size() == pWindow->get_config_number_of_presentable_images());
		// Store the images,
		std::copy(std::begin(swapChainImages), std::end(swapChainImages),
				  std::back_inserter(pWindow->mSwapChainImages));
		// and create one image view per image
		std::transform(std::begin(pWindow->mSwapChainImages), std::end(pWindow->mSwapChainImages),
					   std::back_inserter(pWindow->mSwapChainImageViews),
					   [wnd=pWindow](const auto& image) {
						   auto viewCreateInfo = vk::ImageViewCreateInfo()
							   .setImage(image)
							   .setViewType(vk::ImageViewType::e2D)
							   .setFormat(wnd->swap_chain_image_format().mFormat)
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
						   auto imageView = context().logical_device().createImageView(viewCreateInfo);
						   return imageView;
					   });


		// ============= SYNCHRONIZATION OBJECTS ===========
		// Create them here, already.
		auto numSyncObjects = pWindow->get_config_number_of_concurrent_frames();

		auto fenceInfo = vk::FenceCreateInfo()
			.setFlags(vk::FenceCreateFlagBits::eSignaled);
		auto semaphoreInfo = vk::SemaphoreCreateInfo();
		for (uint32_t i = 0; i < numSyncObjects; ++i) {
			pWindow->mFences.push_back(fence::create(fenceInfo));
			pWindow->mImageAvailableSemaphores.push_back(semaphore::create(semaphoreInfo));
			pWindow->mRenderFinishedSemaphores.push_back(semaphore::create(semaphoreInfo));
		}
	}

	vk::RenderPass vulkan::create_render_pass(image_format pImageFormat, image_format pDepthFormat)
	{
		std::array attachments = {
			// COLOR ATTACHMENT
			vk::AttachmentDescription() 
				.setFormat(pImageFormat.mFormat)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eClear) // what to do with the data in the attachment before rendering and after rendering [5]
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined) // we don't care what previous layout the image was in. The caveat of this special value is that the contents of the image are not guaranteed to be preserved, but that doesn't matter since we're going to clear it anyway. [5]
				.setFinalLayout(vk::ImageLayout::ePresentSrcKHR) //  Images to be presented in the swap chain [5]
			,
			// DEPTH ATTACHMENT
			vk::AttachmentDescription()
				.setFormat(pDepthFormat.mFormat)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eClear) // what to do with the data in the attachment before rendering and after rendering [5]
				.setStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(vk::ImageLayout::eUndefined) // we don't care what previous layout the image was in. The caveat of this special value is that the contents of the image are not guaranteed to be preserved, but that doesn't matter since we're going to clear it anyway. [5]
				.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal) //  Images to be presented in the swap chain [5]
		};

		// Attachment references for subpasses
		auto colorAttachmentRef = vk::AttachmentReference()
			.setAttachment(0u)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal); // We intend to use the attachment to function as a color buffer => this layout will give us the best performance [5]

		auto depthAttachmentRef = vk::AttachmentReference()
			.setAttachment(1u)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto subpassDesc = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1u)
			.setPColorAttachments(&colorAttachmentRef)
			.setPDepthStencilAttachment(&depthAttachmentRef);
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
			.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
			.setPAttachments(attachments.data())
			.setSubpassCount(1u)
			.setPSubpasses(&subpassDesc)
			.setDependencyCount(1u)
			.setPDependencies(&subpassDependency);
		return mLogicalDevice.createRenderPass(renderPassInfo); // TODO: use this
	}

	pipeline vulkan::create_graphics_pipeline_for_window(
		const std::vector<std::tuple<shader_type, shader_handle*>>& pShaderInfos,
		window* pWindow,
		image_format pDepthFormat,
		const vk::VertexInputBindingDescription& pBindingDesc,
		size_t pNumAttributeDesc, const vk::VertexInputAttributeDescription* pAttributeDescDataPtr,
		const std::vector<vk::DescriptorSetLayout>& pDescriptorSetLayouts)
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
			.setVertexAttributeDescriptionCount(static_cast<uint32_t>(pNumAttributeDesc))
			.setPVertexAttributeDescriptions(pAttributeDescDataPtr);

		// HOW TO INTERPRET THE VERTEX INPUT
		auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(VK_FALSE);

		// VIEWPORT AND SCISSORS
		auto scExtent = pWindow->swap_chain_extent();
		auto viewport = vk::Viewport()
			.setX(0.0f)
			.setY(0.0f)
			// Remember that the size of the swap chain and its images may differ from the WIDTH and HEIGHT of the window.The swap chain images will be used as framebuffers later on, so we should stick to their size. [4]
			.setWidth(static_cast<float>(scExtent.width))
			.setHeight(static_cast<float>(scExtent.height))
			// These values must be within the [0.0f, 1.0f] range, but minDepth may be higher than maxDepth. If you aren't doing anything special, then you should stick to the standard values of 0.0f and 1.0f. [4]
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);
		auto scissor = vk::Rect2D()
			.setOffset(vk::Offset2D(0, 0))
			.setExtent(scExtent);

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
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			//.setFrontFace(vk::FrontFace::eClockwise)
			.setDepthBiasEnable(VK_FALSE) // The rasterizer can alter the depth values by adding a constant value or biasing them based on a fragment's slope. This is sometimes used for shadow mapping [4]
			.setDepthBiasConstantFactor(0.0f) // Optional
			.setDepthBiasClamp(0.0f) // Optional
			.setDepthBiasSlopeFactor(0.0f); // Optional

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
			.setSetLayoutCount(static_cast<uint32_t>(pDescriptorSetLayouts.size()))
			.setPSetLayouts(pDescriptorSetLayouts.data())
			.setPushConstantRangeCount(0u)
			.setPPushConstantRanges(nullptr);
		auto pipelineLayout = mLogicalDevice.createPipelineLayout(pipelineLayoutInfo); 


		// CREATE RENDER PASS (for sure, this is the wrong place to do so => refactor!)
		auto renderPass = create_render_pass(pWindow->swap_chain_image_format(), pDepthFormat);


		// DEPTH AND STENCIL STATE
		auto depthStencil = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(true)
			.setDepthWriteEnable(true)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(VK_FALSE)
			.setMinDepthBounds(0.0f)
			.setMaxDepthBounds(1.0f)
			.setStencilTestEnable(VK_FALSE)
			.setFront(vk::StencilOpState())
			.setBack(vk::StencilOpState());

		// MULTISAMPLING
		auto multisamplingInfo = pWindow->get_config_multisample_state_create_info();

		// PIPELINE CREATION
		auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
			.setStageCount(static_cast<uint32_t>(shaderStages.size()))
			.setPStages(shaderStages.data())
			.setPVertexInputState(&vertexInputinfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportInfo)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisamplingInfo)
			.setPDepthStencilState(&depthStencil) // Optional
			.setPColorBlendState(&colorBlendingInfo)
			.setPDynamicState(nullptr) // Optional
			.setLayout(pipelineLayout)
			.setRenderPass(renderPass)
			.setSubpass(0u)
			.setBasePipelineHandle(nullptr) // Optional
			.setBasePipelineIndex(-1); // Optional

		// Create the pipeline, return it and also store the render pass and the pipeline layout in the struct!
		return pipeline(
			pipelineLayout, 
			mLogicalDevice.createGraphicsPipeline(
				nullptr, // references an optional VkPipelineCache object. A pipeline cache can be used to store and reuse data relevant to pipeline creation across multiple calls to vkCreateGraphicsPipelines and even across program executions [5]
				pipelineInfo),
			renderPass);
	}

	pipeline vulkan::create_ray_tracing_pipeline(
		const std::vector<std::tuple<shader_type, shader_handle*>>& pShaderInfos,
		const std::vector<vk::DescriptorSetLayout>& pDescriptorSetLayouts)
	{
		// CREATE PIPELINE
		auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(static_cast<uint32_t>(pDescriptorSetLayouts.size()))
			.setPSetLayouts(pDescriptorSetLayouts.data())
			.setPushConstantRangeCount(0u)
			.setPPushConstantRanges(nullptr);
		auto pipelineLayout = mLogicalDevice.createPipelineLayout(pipelineLayoutInfo);

		// Gather the shader infos
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::transform(std::begin(pShaderInfos), std::end(pShaderInfos),
					   std::back_inserter(shaderStages),
					   [](const auto& tpl) {
						   return vk::PipelineShaderStageCreateInfo()
							   .setStage(convert(std::get<shader_type>(tpl)))
							   .setModule(std::get<shader_handle*>(tpl)->mShaderModule)
							   .setPName("main"); // TODO: support different entry points?!
					   });

		// Create shader groups
		std::array shaderGroups = {
			// group0 = [raygen]
			vk::RayTracingShaderGroupCreateInfoNV()
			.setType(vk::RayTracingShaderGroupTypeNV::eGeneral)
			.setGeneralShader(0) // Just the ray generation shader here, nothing special
			.setClosestHitShader(VK_SHADER_UNUSED_NV)
			.setAnyHitShader(VK_SHADER_UNUSED_NV)
			.setIntersectionShader(VK_SHADER_UNUSED_NV)
			,
			// group1 = [chit]
			vk::RayTracingShaderGroupCreateInfoNV()
			.setType(vk::RayTracingShaderGroupTypeNV::eTrianglesHitGroup)
			.setGeneralShader(VK_SHADER_UNUSED_NV) // Unused because we're using the stock triangles intersection functionality
			.setClosestHitShader(1u) // If we get closest hits, we'd like to have our 2nd entry to the shader table executed
			.setAnyHitShader(VK_SHADER_UNUSED_NV)
			.setIntersectionShader(VK_SHADER_UNUSED_NV)
			,
			// group2 = [chit1]
			vk::RayTracingShaderGroupCreateInfoNV()
			.setType(vk::RayTracingShaderGroupTypeNV::eTrianglesHitGroup)
			.setGeneralShader(VK_SHADER_UNUSED_NV) 
			.setClosestHitShader(2u) 
			.setAnyHitShader(VK_SHADER_UNUSED_NV)
			.setIntersectionShader(VK_SHADER_UNUSED_NV)
			,
			// group3 = [miss0]
			vk::RayTracingShaderGroupCreateInfoNV()
			.setType(vk::RayTracingShaderGroupTypeNV::eGeneral)
			.setGeneralShader(3u) // Ein Miss Shader hat mit einem bestimmten Hit nix zu tun, deswegen is der nicht in der Hit Group mit dabei, sondern ein separater Eintrag
			.setClosestHitShader(VK_SHADER_UNUSED_NV)
			.setAnyHitShader(VK_SHADER_UNUSED_NV)
			.setIntersectionShader(VK_SHADER_UNUSED_NV)
			,
			// group4 = [miss1]
			vk::RayTracingShaderGroupCreateInfoNV()
			.setType(vk::RayTracingShaderGroupTypeNV::eGeneral)
			.setGeneralShader(4u) // Ein Miss Shader hat mit einem bestimmten Hit nix zu tun, deswegen is der nicht in der Hit Group mit dabei, sondern ein separater Eintrag
			.setClosestHitShader(VK_SHADER_UNUSED_NV)
			.setAnyHitShader(VK_SHADER_UNUSED_NV)
			.setIntersectionShader(VK_SHADER_UNUSED_NV)
		};

		// Do it:
		auto pipelineCreateInfo = vk::RayTracingPipelineCreateInfoNV()
			.setStageCount(static_cast<uint32_t>(shaderStages.size()))
			.setPStages(shaderStages.data())
			.setGroupCount(static_cast<uint32_t>(shaderGroups.size()))
			.setPGroups(shaderGroups.data())
			.setMaxRecursionDepth(2u) // Ho ho ho! Wir recursen!
			.setLayout(pipelineLayout);
		return pipeline(
			pipelineLayout,
			context().logical_device().createRayTracingPipelineNV(
				nullptr,						// no pipeline cache
				pipelineCreateInfo,				// pipeline description
				nullptr,						// no allocation callbacks
				context().dynamic_dispatch()));	// dynamic dispatch for extension
	}

	std::vector<framebuffer> vulkan::create_framebuffers(const vk::RenderPass& renderPass, window* pWindow, const image_view& pDepthImageView)
	{
		std::vector<framebuffer> framebuffers;
		auto extent = pWindow->swap_chain_extent();
		for (const auto& imageView : pWindow->swap_chain_image_views()) {
			std::array attachments = { imageView, pDepthImageView.mImageView };
			auto framebufferInfo = vk::FramebufferCreateInfo()
				.setRenderPass(renderPass)
				.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
				.setPAttachments(attachments.data())
				.setWidth(extent.width)
				.setHeight(extent.height)
				.setLayers(1u); // number of layers in image arrays [6]

			framebuffers.push_back(framebuffer{ mLogicalDevice.createFramebuffer( framebufferInfo ) });
		}
		return framebuffers;
	}

	command_pool& vulkan::get_command_pool_for_queue_family(uint32_t pQueueFamilyIndex)
	{
		auto it = std::find_if(std::begin(mCommandPools), std::end(mCommandPools),
							   [pQueueFamilyIndex](const auto& existing) {
								   return existing.mQueueFamilyIndex == pQueueFamilyIndex;
							   });
		if (it == std::end(mCommandPools)) {
			auto commandPoolInfo = vk::CommandPoolCreateInfo()
				.setQueueFamilyIndex(pQueueFamilyIndex)
				.setFlags(vk::CommandPoolCreateFlags()); // Optional
			// Possible values for the flags [7]
			//  - VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
			//  - VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
			mCommandPools.push_back(command_pool::create(pQueueFamilyIndex, commandPoolInfo));
			return mCommandPools.back();
		}
		return *it;
	}

	std::vector<command_buffer> vulkan::create_command_buffers(size_t pCount, uint32_t pQueueFamilyIndex, vk::CommandBufferUsageFlags pUsageFlags)
	{
		auto& pool = get_command_pool_for_queue_family(pQueueFamilyIndex);

		auto bufferAllocInfo = vk::CommandBufferAllocateInfo()
			.setCommandPool(pool.mCommandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary) // TODO: make configurable?!
			.setCommandBufferCount(static_cast<uint32_t>(pCount));

		std::vector<command_buffer> buffers;
		auto tmp = mLogicalDevice.allocateCommandBuffers(bufferAllocInfo);
		std::transform(std::begin(tmp), std::end(tmp),
					   std::back_inserter(buffers),
					   [pUsageFlags](const auto& vkCb) {
						   auto beginInfo = vk::CommandBufferBeginInfo()
							   .setFlags(pUsageFlags)
							   .setPInheritanceInfo(nullptr);
						   return command_buffer{ beginInfo, vkCb };
					   });
		return buffers;
	}

	std::vector<command_buffer> vulkan::create_command_buffers_for_graphics(size_t pCount, vk::CommandBufferUsageFlags pUsageFlags)
	{
		return create_command_buffers(pCount, graphics_queue_index(), pUsageFlags);
	}

	std::vector<command_buffer> vulkan::create_command_buffers_for_transfer(size_t pCount, vk::CommandBufferUsageFlags pUsageFlags)
	{
		return create_command_buffers(pCount, transfer_queue_index(), pUsageFlags);
	}

	std::vector<command_buffer> vulkan::create_command_buffers_for_presentation(size_t pCount, vk::CommandBufferUsageFlags pUsageFlags)
	{
		return create_command_buffers(pCount, presentation_queue_index(), pUsageFlags);
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

	void vulkan::set_sharing_mode_for_transfer(vk::BufferCreateInfo& pCreateInfo)
	{
		if (graphics_queue_index() == transfer_queue_index()) {
			pCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
		}
		else {
			pCreateInfo.setSharingMode(vk::SharingMode::eConcurrent);
			pCreateInfo.setQueueFamilyIndexCount(static_cast<uint32_t>(mAllUsedQueueFamilyIndices.size()));
			pCreateInfo.setPQueueFamilyIndices(mAllUsedQueueFamilyIndices.data());
		}
	}

	descriptor_pool& vulkan::get_descriptor_pool()
	{
		if (mDescriptorPools.size() == 0) {

			std::array poolSizes = {
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eUniformBuffer)
					.setDescriptorCount(1024u) // TODO: is that a good pool size? and what to do beyond that number?
				,
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eCombinedImageSampler)
					.setDescriptorCount(1024u) // TODO: is that a good pool size? and what to do beyond that number?
				,
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eStorageImage)
					.setDescriptorCount(1024u) // TODO: is that a good pool size? and what to do beyond that number?
				,
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eAccelerationStructureNV)
					.setDescriptorCount(1024u) // TODO: is that a good pool size? and what to do beyond that number?
				,
				// Added because of IMGUI:
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eSampler)
					.setDescriptorCount(1024u)
				,
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eSampledImage)
					.setDescriptorCount(1024u)
				,
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eUniformTexelBuffer)
					.setDescriptorCount(1024u)
				,
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eStorageTexelBuffer)
					.setDescriptorCount(1024u)
				,
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eStorageBuffer)
					.setDescriptorCount(1024u)
				,
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eUniformBufferDynamic)
					.setDescriptorCount(1024u)
				,
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eStorageBufferDynamic)
					.setDescriptorCount(1024u)
				,
				vk::DescriptorPoolSize()
					.setType(vk::DescriptorType::eInputAttachment)
					.setDescriptorCount(1024u)
			};

			auto poolInfo = vk::DescriptorPoolCreateInfo()
				.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
				.setPPoolSizes(poolSizes.data())
				.setMaxSets(128u) // TODO: is that a good max sets-number? and what to do beyond that number?
				.setFlags(vk::DescriptorPoolCreateFlags()); // The structure has an optional flag similar to command pools that determines if individual descriptor sets can be freed or not: VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT. We're not going to touch the descriptor set after creating it, so we don't need this flag. [10]
			return mDescriptorPools.emplace_back(logical_device().createDescriptorPool(poolInfo));
		}
		return mDescriptorPools[0];
	}

	std::vector<descriptor_set> vulkan::create_descriptor_set(std::vector<vk::DescriptorSetLayout> pData)
	{
		auto allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(get_descriptor_pool().mDescriptorPool)
			.setDescriptorSetCount(static_cast<uint32_t>(pData.size()))
			.setPSetLayouts(pData.data());
		auto descriptorSets = logical_device().allocateDescriptorSets(allocInfo); // The call to vkAllocateDescriptorSets will allocate descriptor sets, each with one uniform buffer descriptor. [10]
		std::vector<descriptor_set> result;
		std::transform(std::begin(descriptorSets), std::end(descriptorSets),
					   std::back_inserter(result),
					   [](const auto& vkDescSet) {
						   return descriptor_set(vkDescSet);
					   });
		return result;
	}

	bool vulkan::is_format_supported(vk::Format pFormat, vk::ImageTiling pTiling, vk::FormatFeatureFlags pFormatFeatures)
	{
		auto formatProps = mPhysicalDevice.getFormatProperties(pFormat);
		if (pTiling == vk::ImageTiling::eLinear 
			&& (formatProps.linearTilingFeatures & pFormatFeatures) == pFormatFeatures) {
			return true;
		}
		else if (pTiling == vk::ImageTiling::eOptimal 
				 && (formatProps.optimalTilingFeatures & pFormatFeatures) == pFormatFeatures) {
			return true;
		} 
		return false;
	}


	vk::PhysicalDeviceRayTracingPropertiesNV vulkan::get_ray_tracing_properties()
	{
		vk::PhysicalDeviceRayTracingPropertiesNV rtProps;
		vk::PhysicalDeviceProperties2 props2;
		props2.pNext = &rtProps;
		mPhysicalDevice.getProperties2(&props2);
		return rtProps;
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
	// [10] Vulkan Tutorial, Descriptor pool and sets, https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets
}
