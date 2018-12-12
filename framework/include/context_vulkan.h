#pragma once

// DEFINES:
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define VK_USE_PLATFORM_WIN32_KHR

// INCLUDES:
#include <vulkan/vulkan.hpp>
#include "context_generic_glfw_types.h"
#include "context_vulkan_types.h"
#include "context_generic_glfw.h"

namespace cgb
{
	/**	@brief Context for Vulkan
	 *
	 *	This context abstracts calls to the Vulkan API, for environment-related
	 *	stuff, like window creation etc.,  it relies on GLFW and inherits
	 *	@ref generic_glfw.
	 */
	class vulkan : public generic_glfw
	{
	public:
		vulkan();
		vulkan(const vulkan&) = delete;
		vulkan(vulkan&&) = delete;
		vulkan& operator=(const vulkan&) = delete;
		vulkan& operator=(vulkan&&) = delete;
		virtual ~vulkan();

		window* create_window(const window_params&);

		texture_handle create_texture()
		{
			return texture_handle();
		}

		void destroy_texture(const texture_handle& pHandle)
		{
		}

	private:
		/** Queries the instance layer properties for validation layers 
		 *  and returns true if a layer with the given name could be found.
		 *  Returns false if not found. 
		 */
		static bool is_validation_layer_supported(const char* pName);

		/**	Compiles all those entries from @ref settings::gValidationLayersToBeActivated into
		 *	an array which are supported by the instance. A warning will be issued for those
		 *	entries which are not supported.
		 */
		auto assemble_validation_layers();

		/** Create a new vulkan instance with all the application information and
		 *	also set the required instance extensions which GLFW demands.
		 */
		void create_instance();

		/** Method which handles debug callbacks from the validation layers */
		static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);

		/** Set up the debug callbacks, i.e. hook into vk to have @ref vk_debug_callback called */
		void setup_vk_debug_callback();

		/** Creates a surface for the given window */
		vk::SurfaceKHR* create_surface_for_window(window* pWindow);

		/** Gets the surface assigned to the given window.
		 *	@return Pointer to the surface or nullptr if not found
		 */
		vk::SurfaceKHR* get_surface_for_window(window* pWindow);
		
		/** Gets the window assigned to the given surface.
		 *	@return Pointer to the window or nullptr if not found
		 */
		window* get_window_for_surface(const vk::SurfaceKHR pSurface);

		/** Checks whether the given physical device supports all the required extensions,
		 *	namely those stored in @ref settings::gRequiredDeviceExtensions. 
		 *	Returns true if it does, false otherwise.
		 */
		static bool supports_all_required_extensions(const vk::PhysicalDevice& device);

		/** Pick the physical device which looks to be the most promising one */
		void pick_physical_device();

		/**	Finds all queue families which support the given @ref vk::QueueFlagBits.
		 *	All which support it are returned as a vector of tuples of indices and data.
		 *	The index is important for further vk-calls and is stored in the first element
		 *	of the tuple, i.e. use @ref std::get<0>() to get the index, @ref std::get<1>() 
		 *	for the data
		 */
		auto find_queue_families_with_flags(vk::QueueFlagBits requiredFlags);

		// TODO: double-check and comment
		void create_logical_device();

		// TODO: double-check and comment
		void get_graphics_queue();

	private:
		vk::Instance mInstance;
		VkDebugUtilsMessengerEXT mDebugCallbackHandle;
		std::vector<std::unique_ptr<std::tuple<window*, vk::SurfaceKHR>>> mSurfaces;
		vk::PhysicalDevice mPhysicalDevice;
		vk::Device mLogicalDevice;
		vk::Queue mGraphicsQueue;
	};
}
