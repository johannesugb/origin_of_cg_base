#pragma once

// DEFINES:
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define VK_USE_PLATFORM_WIN32_KHR

// INCLUDES:
#include <vulkan/vulkan.hpp>
#include "context_vulkan_types.h"
#include "context_generic_glfw.h"

namespace cgb
{
	// =============================== type aliases =================================
	using swap_chain_data_ptr = std::unique_ptr<swap_chain_data>;

	// ============================== VULKAN CONTEXT ================================
	/**	@brief Context for Vulkan
	 *
	 *	This context abstracts calls to the Vulkan API, for environment-related
	 *	stuff, like window creation etc.,  it relies on GLFW and inherits
	 *	@ref generic_glfw.
	 */
	class vulkan : public generic_glfw
	{
		friend struct texture_handle;
		friend struct image_format;
		friend struct swap_chain_data;
		friend struct shader_handle;
		friend struct pipeline;
		friend struct framebuffer;
		friend struct command_pool;
		friend struct command_buffer;
	public:
		vulkan();
		vulkan(const vulkan&) = delete;
		vulkan(vulkan&&) = delete;
		vulkan& operator=(const vulkan&) = delete;
		vulkan& operator=(vulkan&&) = delete;
		virtual ~vulkan();

		window* create_window(const window_params&, const swap_chain_params&);

		texture_handle create_texture()
		{
			return texture_handle();
		}

		void destroy_texture(const texture_handle& pHandle)
		{
		}

		void draw_triangle(const pipeline& pPipeline, const command_buffer& pCommandBuffer);

	public: // TODO: private
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
		vk::SurfaceKHR create_surface_for_window(const window* pWindow);

		/** 
		 *	@return Pointer to the tuple or nullptr if not found
		 */
		swap_chain_data* get_surf_swap_tuple_for_window(const window* pWindow);
		
		/**
		 *	@return Pointer to the tuple or nullptr if not found
		 */
		swap_chain_data* get_surf_swap_tuple_for_surface(const vk::SurfaceKHR& pSurface);

		/**
		 *	@return Pointer to the tuple or nullptr if not found
		 */
		swap_chain_data* get_surf_swap_tuple_for_swap_chain(const vk::SwapchainKHR& pSwapChain);

		/** Returns a vector containing all elements from @ref sRequiredDeviceExtensions
		 *  and settings::gRequiredDeviceExtensions
		 */
		static std::vector<const char*> get_all_required_device_extensions();
		
		/** Checks whether the given physical device supports all the required extensions,
		 *	namely those stored in @ref settings::gRequiredDeviceExtensions. 
		 *	Returns true if it does, false otherwise.
		 */
		static bool supports_all_required_extensions(const vk::PhysicalDevice& device);

		/** Pick the physical device which looks to be the most promising one */
		void pick_physical_device();


		/**	Finds all queue families which support certain criteria which are defined by the parameters.
		 *	@param pRequiredFlags	If set, a queue family must support the set flags
		 *	@param pSurface			If set, the queue family must support the given surface
		 *	@return		All which support them are returned as a vector of tuples of indices and data.
		 *				The index is important for further vk-calls and is stored in the first element
		 *				of the tuple, i.e. use @ref std::get<0>() to get the index, @ref std::get<1>() 
		 *				for the data
		 */
		auto find_queue_families_for_criteria(std::optional<vk::QueueFlagBits> pRequiredFlags, std::optional<vk::SurfaceKHR> pSurface);

		/**
		 *
		 */
		vk::Device create_logical_device(vk::SurfaceKHR pSurface);

		/** Creates the swap chain for the given window and surface with the given parameters
		 *	@param pWindow		[in] The window to create the swap chain for
		 *	@param pSurface		[in] the surface to create the swap chain for
		 *	@param pParams		[in] swap chain creation parameters
		 *	@return				A newly created swap chain
		 */
		swap_chain_data create_swap_chain(const window* pWindow, const vk::SurfaceKHR& pSurface, const swap_chain_params& pParams);

		/** TODO: TBD */
		vk::RenderPass create_render_pass(image_format pImageFormat);

		/** TODO: TBD */
		pipeline create_graphics_pipeline_for_window(const std::vector<std::tuple<shader_type, shader_handle*>>& pShaderInfos, const window* pWindow);
		/** TODO: TBD */
		pipeline create_graphics_pipeline_for_swap_chain(const std::vector<std::tuple<shader_type, shader_handle*>>& pShaderInfos, const swap_chain_data& pSwapChainData);

		std::vector<framebuffer> create_framebuffers(const vk::RenderPass& renderPass, const window* pWindow);
		std::vector<framebuffer> create_framebuffers(const vk::RenderPass& renderPass, const swap_chain_data& pSwapChainData);

		command_pool create_command_pool();

		std::vector<command_buffer> create_command_buffers(uint32_t pCount, const command_pool& pCommandPool);
		
	private:
		static std::vector<const char*> sRequiredDeviceExtensions;
		vk::Instance mInstance;
		VkDebugUtilsMessengerEXT mDebugCallbackHandle;
		std::vector<swap_chain_data_ptr> mSurfSwap;
		vk::PhysicalDevice mPhysicalDevice;
		vk::Device mLogicalDevice;
		vk::Queue mGraphicsQueue;
		vk::Queue mPresentQueue;
	};
}
