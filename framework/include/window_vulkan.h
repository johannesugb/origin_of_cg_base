#pragma once
#include <vulkan/vulkan.hpp>
#include "context_generic_glfw_types.h"
#include "window_base.h"

namespace cgb
{
	class window : public window_base
	{
		friend class generic_glfw;
		friend class vulkan;
	public:
		window();
		~window();
		window(const window&) = delete;
		window(window&&) noexcept;
		window& operator =(const window&) = delete;
		window& operator =(window&&) noexcept;

		/** Request a framebuffer for this window which is capable of sRGB formats */
		void request_srgb_framebuffer(bool pRequestSrgb);

		/** Sets the presentation mode for this window's swap chain. */
		void set_presentaton_mode(cgb::presentation_mode pMode);

		/** Sets the number of samples for MSAA */
		void set_number_of_samples(int pNumSamples);

		/** Creates or opens the window */
		void open();

		/** Gets the requested surface format for the given surface.
		 *	A default value will be set if no other value has been configured.
		 */
		vk::SurfaceFormatKHR get_surface_format(const vk::SurfaceKHR& surface);

		/** Gets the requested presentation mode for the given surface.
		 *	A default value will be set if no other value has been configured.
		 */
		vk::PresentModeKHR get_presentation_mode(const vk::SurfaceKHR& surface);

		/**	Gets the number of samples that has been configured.
		 *	A default value will be set if no other value has been configured.
		 */
		vk::SampleCountFlagBits get_number_of_samples();

		/** Gets the multisampling-related config info struct for the Vk-pipeline config.
		 *	A default value will be set if no other value has been configured.
		 */
		vk::PipelineMultisampleStateCreateInfo get_multisample_state_create_info();

	protected:
		// A function which returns the surface format for this window's surface
		std::function<vk::SurfaceFormatKHR(const vk::SurfaceKHR&)> mSurfaceFormatSelector;

		// A function which returns the desired presentation mode for this window's surface
		std::function<vk::PresentModeKHR(const vk::SurfaceKHR&)> mPresentationModeSelector;

		// A function which returns the MSAA sample count for this window's surface
		std::function<vk::SampleCountFlagBits()> mNumberOfSamplesGetter;

		// A funciton which returns the MSAA state for this window's surface
		std::function<vk::PipelineMultisampleStateCreateInfo()> mMultisampleCreateInfoBuilder;

	protected: /* Data for this window */

		// These are actually the data from the swap_chain_data struct, but actually,
		// that's all tied to the window. So, why not have that all here directly?!
		vk::SurfaceKHR mSurface;
		vk::SwapchainKHR mSwapChain;
		image_format mSwapChainImageFormat;
		vk::Extent2D mSwapChainExtent;
		std::vector<vk::Image> mSwapChainImages;
		std::vector<vk::ImageView> mSwapChainImageViews;

		// Synchronization objects - we need those all per window, don't we?!
		static size_t sActualMaxFramesInFlight; // Don't they depend on the PRESENTATION MODE?! 
		size_t mFrameCounter;
		std::vector<vk::Semaphore> mImageAvailableSemaphores; // GPU-GPU synchronization
		std::vector<vk::Semaphore> mRenderFinishedSemaphores; // GPU-GPU synchronization
		std::vector<vk::Fence> mInFlightFences;  // CPU-GPU synchronization

		// The backbuffer of this window
		framebuffer mBackBuffer;

		// The render pass for this window's UI calls
		vk::RenderPass mUiRenderPass;
	};
}
