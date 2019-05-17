#pragma once
#include <vulkan/vulkan.hpp>
#include "window_vulkan.h"
#include "context_vulkan_types.h"
#include "context_generic_glfw.h"
#include "window_base.h"

namespace cgb
{
	class window : public window_base
	{
		friend class generic_glfw;
		friend class vulkan;
	public:
		struct frame_sync_data
		{
			fence& mFence;
			semaphore& mImageAvailableSemaphore;
			semaphore& mRenderFinishedSemaphore;

		};


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

		/** Sets the number of presentable images for a swap chain */
		void set_number_of_presentable_images(uint32_t pNumImages);

		/** Sets the number of images which can be rendered into concurrently,
		 *	i.e. the number of "frames in flight"
		 */
		void set_number_of_concurrent_frames(uint32_t pNumConcurrent);

		/** Creates or opens the window */
		void open();

		/** Gets the requested surface format for the given surface.
		 *	A default value will be set if no other value has been configured.
		 */
		vk::SurfaceFormatKHR get_config_surface_format(const vk::SurfaceKHR& surface);

		/** Gets the requested presentation mode for the given surface.
		 *	A default value will be set if no other value has been configured.
		 */
		vk::PresentModeKHR get_config_presentation_mode(const vk::SurfaceKHR& surface);

		/**	Gets the number of samples that has been configured.
		 *	A default value will be set if no other value has been configured.
		 */
		vk::SampleCountFlagBits get_config_number_of_samples();

		/** Gets the multisampling-related config info struct for the Vk-pipeline config.
		 *	A default value will be set if no other value has been configured.
		 */
		vk::PipelineMultisampleStateCreateInfo get_config_multisample_state_create_info();

		/** Get the minimum number of concurrent/presentable images for a swap chain.
		*	If no value is set, the surfaces minimum number + 1 will be returned.
		*/
		uint32_t get_config_number_of_presentable_images();

		/** Get the number of concurrent frames.
		*	If no value is explicitely set, the same number as the number of presentable images will be returned.
		*/
		uint32_t get_config_number_of_concurrent_frames();

		/** Gets this window's surface */
		const auto& surface() const { 
			return mSurface; 
		}
		/** Gets this window's swap chain */
		const auto& swap_chain() const { 
			return mSwapChain; 
		}
		/** Gets this window's swap chain's image format */
		const auto& swap_chain_image_format() const { 
			return mSwapChainImageFormat; 
		}
		/** Gets this window's swap chain's dimensions */
		auto swap_chain_extent() const {
			return mSwapChainExtent; 
		}
		/** Gets a collection containing all this window's swap chain images. */
		const auto& swap_chain_images() { 
			return mSwapChainImages;
		}
		/** Gets this window's swap chain's image at the specified index. */
		const auto& swap_chain_image_at_index(size_t pIdx) { 
			return mSwapChainImages[pIdx]; 
		}
		/** Gets a collection containing all this window's swap chain image views. */
		const auto& swap_chain_image_views() { 
			return mSwapChainImageViews; 
		}
		/** Gets this window's swap chain's image view at the specified index. */
		const auto& swap_chain_image_view_at_index(size_t pIdx) { 
			return mSwapChainImageViews[pIdx]; 
		}

		/** Gets the number of how many images there are in the swap chain. */
		auto number_of_swapchain_images() const {
			return mSwapChainImageViews.size(); 
		}
		/** Gets the number of how many frames are (potentially) concurrently rendered into. */
		auto number_of_concurrent_frames() const { 
			return mFences.size(); 
		}

		/** Gets the current frame index. */
		auto current_frame() const { 
			return mCurrentFrame; 
		}
		/** Helper function which calculates the image index for the given frame index. 
		 *	@param pFrameIndex Frame index which to calculate the corresponding image index for.
		 *	@return Returns the image index for the given frame index.
		 */
		auto calculate_image_index_for_frame(int64_t pFrameIndex) const { 
			return pFrameIndex % number_of_swapchain_images(); 
		}
		/** Helper function which calculates the sync index for the given frame index. 
		*	@param pFrameIndex Frame index which to calculate the corresponding image index for.
		*	@return Returns the sync index for the given frame index.
		*/
		auto calculate_sync_index_for_frame(int64_t pFrameIndex) const {
			return pFrameIndex % number_of_concurrent_frames(); 
		}

		/** Returns the image index for the requested frame.
		 *	@param pCurrentFrameOffset	Leave at the default of 0 to request the image index for the current frame.
		 *								Specify a negative offset to refer to a previous frame.
		 */
		auto image_index_for_frame(int64_t pCurrentFrameOffset = 0) const { 
			return calculate_image_index_for_frame(static_cast<int64_t>(current_frame()) + pCurrentFrameOffset); 
		}
		/** Returns the sync index for the requested frame.
		*	@param pCurrentFrameOffset	Leave at the default of 0 to request the image index for the current frame.
		*								Specify a negative offset to refer to a previous frame.
		*/
		auto sync_index_for_frame(int64_t pCurrentFrameOffset = 0) const { 
			return calculate_sync_index_for_frame(static_cast<int64_t>(current_frame()) + pCurrentFrameOffset); 
		}
		
		/** Returns the swap chain image for the requested frame.
		*	@param pCurrentFrameOffset	Leave at the default of 0 to request the image index for the current frame.
		*								Specify a negative offset to refer to a previous frame.
		*/
		const auto& image_for_frame(int64_t pCurrentFrameOffset = 0) const {
			return mSwapChainImages[image_index_for_frame(pCurrentFrameOffset)];
		}
		/** Returns the swap chain image view for the requested frame.
		*	@param pCurrentFrameOffset	Leave at the default of 0 to request the image index for the current frame.
		*								Specify a negative offset to refer to a previous frame.
		*/
		const auto& image_view_for_frame(int64_t pCurrentFrameOffset = 0) const {
			return mSwapChainImageViews[image_index_for_frame(pCurrentFrameOffset)];
		}
		/** Returns the fence for the requested frame.
		*	@param pCurrentFrameOffset	Leave at the default of 0 to request the image index for the current frame.
		*								Specify a negative offset to refer to a previous frame.
		*/
		const auto& fence_for_frame(int64_t pCurrentFrameOffset = 0) const {
			return mFences[sync_index_for_frame(pCurrentFrameOffset)];
		}
		/** Returns the "image available"-semaphore for the requested frame.
		*	@param pCurrentFrameOffset	Leave at the default of 0 to request the image index for the current frame.
		*								Specify a negative offset to refer to a previous frame.
		*/
		const auto& image_available_semaphore_for_frame(int64_t pCurrentFrameOffset = 0) const {
			return mImageAvailableSemaphores[sync_index_for_frame(pCurrentFrameOffset)];
		}
		/** Returns the "render finished"-semaphore for the requested frame.
		*	@param pCurrentFrameOffset	Leave at the default of 0 to request the image index for the current frame.
		*								Specify a negative offset to refer to a previous frame.
		*/
		const auto& render_finished_semaphore_for_frame(int64_t pCurrentFrameOffset = 0) const {
			return mRenderFinishedSemaphores[sync_index_for_frame(pCurrentFrameOffset)];
		}

		void set_extra_semaphore_dependency_for_frame(semaphore pSemaphore, uint64_t pFrameId);

		std::vector<semaphore> remove_all_extra_semaphore_dependencies_for_frame(uint64_t pFrameId);

		std::vector<semaphore> set_num_extra_semaphores_to_generate_per_frame(uint32_t pNumExtraSemaphores);

		template<typename CBT, typename... CommandBufferT>
		void render_frame(CBT pCommandBuffer, CommandBufferT... pCommandBuffers)
		{
			auto& fence = fence_for_frame();
			cgb::context().logical_device().waitForFences(1u, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
			cgb::context().logical_device().resetFences(1u, &fence);

			uint32_t imageIndex;
			cgb::context().logical_device().acquireNextImageKHR(
				swap_chain().m_swapchainKHR, // the swap chain from which we wish to acquire an image [1]
				std::numeric_limits<uint64_t>::max(), // a timeout in nanoseconds for an image to become available. Using the maximum value of a 64 bit unsigned integer disables the timeout. [1]
				image_available_semaphore_for_frame(), // The next two parameters specify synchronization objects that are to be signaled when the presentation engine is finished using the image [1]
				nullptr,
				&imageIndex); // a variable to output the index of the swap chain image that has become available. The index refers to the VkImage in our swapChainImages array. We're going to use that index to pick the right command buffer. [1]

			std::array<CBT, sizeof...(pCommandBuffers) + 1> cmdBuffers = { pCommandBuffer, pCommandBuffers... };

			std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
			auto submitInfo = vk::SubmitInfo()
				.setWaitSemaphoreCount(1u)
				.setPWaitSemaphores(image_available_semaphore_for_frame())
				.setPWaitDstStageMask(waitStages.data())
				.setCommandBufferCount(cmdBuffers.size())
				.setPCommandBuffers(&cmdBuffers[0].mCommandBuffer)
				.setSignalSemaphoreCount(1u)
				.setPSignalSemaphores(render_finished_semaphore_for_frame());
			// TODO: This only works because we are using cgb::varying_update_timer which makes a call to render() in each and every frame
			cgb::context().graphics_queue().submit(1u, &submitInfo, fence_for_frame());

			auto presentInfo = vk::PresentInfoKHR()
				.setWaitSemaphoreCount(1u)
				.setPWaitSemaphores(render_finished_semaphore_for_frame())
				.setSwapchainCount(1u)
				.setPSwapchains(swap_chain().m_swapchainKHR)
				.setPImageIndices(&imageIndex)
				.setPResults(nullptr);
			cgb::context().presentation_queue().presentKHR(presentInfo);
			// TODO: das ist alles sicher ganz ganz falsch, vermutlich, aber das konzept wäre quasi so richtig, vielleicht
		}

	protected:
		

#pragma region configuration properties
		// A function which returns the surface format for this window's surface
		std::function<vk::SurfaceFormatKHR(const vk::SurfaceKHR&)> mSurfaceFormatSelector;

		// A function which returns the desired presentation mode for this window's surface
		std::function<vk::PresentModeKHR(const vk::SurfaceKHR&)> mPresentationModeSelector;

		// A function which returns the MSAA sample count for this window's surface
		std::function<vk::SampleCountFlagBits()> mNumberOfSamplesGetter;

		// A function which returns the MSAA state for this window's surface
		std::function<vk::PipelineMultisampleStateCreateInfo()> mMultisampleCreateInfoBuilder;

		// A function which returns the desired number of presentable images in the swap chain
		std::function<uint32_t()> mNumberOfPresentableImagesGetter;

		// A function which returns the number of images which can be rendered into concurrently
		// According to this number, the number of semaphores and fences will be determined.
		std::function<uint32_t()> mNumberOfConcurrentFramesGetter;
#pragma endregion

#pragma region swap chain data for this window's surface
		// The frame counter/frame id/frame index/current frame number
		uint64_t mCurrentFrame;

		// The window's surface
		vk::SurfaceKHR mSurface;
		// The swap chain for this surface
		vk::SwapchainKHR mSwapChain;
		// The swap chain's image format
		image_format mSwapChainImageFormat;
		// The swap chain's extent
		vk::Extent2D mSwapChainExtent;
		// All the images of the swap chain
		std::vector<vk::Image> mSwapChainImages;
		// All the image views of the swap chain
		std::vector<vk::ImageView> mSwapChainImageViews;
#pragma endregion

#pragma region indispensable sync elements
		// Fences to synchronize between frames (CPU-GPU synchronization)
		std::vector<fence> mFences; 
		// Semaphores to wait for an image to become available (GPU-GPU synchronization) // TODO: true?
		std::vector<semaphore> mImageAvailableSemaphores; 
		// Semaphores to wait for rendering to finish (GPU-GPU synchronization) // TODO: true?
		std::vector<semaphore> mRenderFinishedSemaphores; 
#pragma endregion

#pragma region extra sync elements, i.e. exta semaphores
		// Extra semaphores for frames.
		// The first element in the tuple refers to the frame id which is affected.
		// The second element in the is the semaphore to wait on.
		// Extra dependency semaphores will be waited on along with the mImageAvailableSemaphores
		std::vector<std::tuple<uint64_t, semaphore>> mExtraSemaphoreDependencies;
		 
		// Number of extra semaphores to generate per frame upon fininshing the rendering of a frame
		uint32_t mNumExtraRenderFinishedSemaphoresPerFrame;

		// Contains the extra semaphores to be signalled per frame
		// The length of this vector will be: number_of_concurrent_frames() * mNumExtraSemaphoresPerFrame
		// These semaphores will be signalled together with the mRenderFinishedSemaphores
		std::vector<semaphore> mExtraRenderFinishedSemaphores;
#pragma endregion

		// The backbuffer of this window
		framebuffer mBackBuffer;

		// The render pass for this window's UI calls
		vk::RenderPass mUiRenderPass;
	};
}
