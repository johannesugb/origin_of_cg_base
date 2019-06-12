#pragma once
#include <vulkan/vulkan.hpp>
#include "context_generic_glfw_types.h"
#include "window_base.h"

namespace cgb
{
	/** Represents a native texture handle for the Vulkan context */
	struct texture_handle
	{
		int m_vulkan_specific_handle;
	};

	/** Represents one specific native image format for the Vulkan context */
	struct image_format
	{
		image_format() noexcept;
		image_format(const vk::Format& pFormat) noexcept;
		image_format(const vk::SurfaceFormatKHR& pSrfFrmt) noexcept;

		vk::Format mFormat;
	};

	/** Returns true if the given image format is a sRGB format
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_srgb_format(const image_format& pImageFormat);
	/** Returns true if the given image format stores the color channels in uint8-type storage
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_uint8_format(const image_format& pImageFormat);
	/** Returns true if the given image format stores the color channels in int8-type storage
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_int8_format(const image_format& pImageFormat);
	/** Returns true if the given image format stores the color channels in uint16-type storage
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_uint16_format(const image_format& pImageFormat);
	/** Returns true if the given image format stores the color channels in int16-type storage
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_int16_format(const image_format& pImageFormat);
	/** Returns true if the given image format stores the color channels in uint32-type storage
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_uint32_format(const image_format& pImageFormat);
	/** Returns true if the given image format stores the color channels in int32-type storage
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_int32_format(const image_format& pImageFormat);
	/** Returns true if the given image format stores the color channels in float16-type storage
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_float16_format(const image_format& pImageFormat);
	/** Returns true if the given image format stores the color channels in float32-type storage
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_float32_format(const image_format& pImageFormat);
	/** Returns true if the given image format stores the color channels in float64-type storage
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_float64_format(const image_format& pImageFormat);
	/** Returns true if the given image's color channels are ordered like follows: RGB
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_rgb_format(const image_format& pImageFormat);
	/** Returns true if the given image's color channels are ordered like follows: RGBA
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_rgba_format(const image_format& pImageFormat);
	/** Returns true if the given image's color channels are ordered like follows: ARGB
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_argb_format(const image_format& pImageFormat);
	/** Returns true if the given image's color channels are ordered like follows: BGR
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_bgr_format(const image_format& pImageFormat);
	/** Returns true if the given image's color channels are ordered like follows: BGRA
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_bgra_format(const image_format& pImageFormat);
	/** Returns true if the given image's color channels are ordered like follows: ABGR
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool is_abgr_format(const image_format& pImageFormat);
	/** Returns true if the given image format is a depth/depth-stencil format and has a stencil component.
	 *	Please note: This function does not guarantee completeness for all formats, i.e. false negatives must be expected. */
	extern bool has_stencil_component(const image_format& pImageFormat);

	

	/** Struct which stores data for a swap chain */
	struct swap_chain_data
	{
		window* mWindow;
		window* mWindow;
		vk::SurfaceKHR mSurface;
		vk::SwapchainKHR mSwapChain;
		image_format mSwapChainImageFormat;
		vk::Extent2D mSwapChainExtent;
		std::vector<vk::Image> mSwapChainImages;
		std::vector<vk::ImageView> mSwapChainImageViews;
	};

	/** Represents a shader program handle for the Vulkan context */
	struct shader_handle
	{
		shader_handle() noexcept;
		shader_handle(const vk::ShaderModule&) noexcept;
		shader_handle(const shader_handle&) = delete;
		shader_handle(shader_handle&&) noexcept;
		shader_handle& operator=(const shader_handle&) = delete;
		shader_handle& operator=(shader_handle&&) noexcept;
		~shader_handle();

		static shader_handle create_from_binary_code(const std::vector<char>& code);

		vk::ShaderModule mShaderModule;
	};

	/** Represents one specific type of shader */
	enum struct shader_type
	{
		/** Vertex Shader */
		vertex,
		/** Tessellation Control Shader */
		tessellation_control,
		/** Tessellation Evaluation Shader */
		tessellation_evaluation,
		/** Geometry Shader */
		geometry,
		/** Fragment Shader */
		fragment,
		/** Compute Shader */
		compute,
		/** Ray Generation Shader (Nvidia RTX) */
		ray_generation,
		/** Ray Tracing Any Hit Shader  (Nvidia RTX) */
		any_hit,
		/** Ray Tracing Closest Hit Shader (Nvidia RTX) */
		closest_hit,
		/** Ray Tracing Miss Shader (Nvidia RTX) */
		miss,
		/** Ray Tracing Intersection Shader (Nvidia RTX) */
		intersection,
		/** Ray Tracing Callable Shader (Nvidia RTX) */
		callable,
		/** Task Shader (Nvidia Turing)  */
		task,
		/** Mesh Shader (Nvidia Turing)  */
		mesh
	};

	/** Converts a cgb::shader_type to the vulkan-specific vk::ShaderStageFlagBits type */
	extern vk::ShaderStageFlagBits convert(shader_type p);

	/** Represents data for a vulkan graphics pipeline 
	 *	The data held by such a struct is a triple of:
	 *    - render pass
	 *    - pipeline layout
	 *    - pipeline handle
	 */
	struct pipeline
	{
		pipeline() noexcept;
		pipeline(const vk::PipelineLayout& pPipelineLayout, const vk::Pipeline& pPipeline, vk::RenderPass pRenderPass = nullptr) noexcept;
		pipeline(const pipeline&) = delete;
		pipeline(pipeline&&) noexcept;
		pipeline& operator=(const pipeline&) = delete;
		pipeline& operator=(pipeline&&) noexcept;
		~pipeline();

		vk::RenderPass mRenderPass;
		vk::PipelineLayout mPipelineLayout;
		vk::Pipeline mPipeline;
	};

	/** Represents a Vulkan framebuffer, holds the native handle and takes
	 *	care about lifetime management of the native handles.
	 */
	struct framebuffer
	{
		framebuffer() noexcept;
		framebuffer(const vk::Framebuffer&) noexcept;
		framebuffer(const framebuffer&) = delete;
		framebuffer(framebuffer&&) noexcept;
		framebuffer& operator=(const framebuffer&) = delete;
		framebuffer& operator=(framebuffer&&) noexcept;
		~framebuffer();

		vk::Framebuffer mFramebuffer;
	};

	/** Represents a Vulkan command pool, holds the native handle and takes
	 *	care about lifetime management of the native handles.
	 *	Also contains the queue family index it has been created for and is 
	 *  intended to be used with:
	 *	"All command buffers allocated from this command pool must be submitted 
	 *	 on queues from the same queue family." [+]
	 *	
	 *	[+]: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkCommandPoolCreateInfo.html
	 */
	struct command_pool
	{
		command_pool() noexcept;
		command_pool(uint32_t, const vk::CommandPoolCreateInfo&, const vk::CommandPool&) noexcept;
		command_pool(const command_pool&) = delete;
		command_pool(command_pool&&) noexcept;
		command_pool& operator=(const command_pool&) = delete;
		command_pool& operator=(command_pool&&) noexcept;
		~command_pool();

		static command_pool create(uint32_t pQueueFamilyIndex, const vk::CommandPoolCreateInfo& pCreateInfo);

		uint32_t mQueueFamilyIndex;
		vk::CommandPoolCreateInfo mCreateInfo;
		vk::CommandPool mCommandPool;
	};
	

	/**	Create (multiple) buffers which are all created with exclusive access for a queue.
	 *	If different queues are being used, ownership has to be transferred explicitely.
	 */
	template <typename Cfg>
	cgb::buffer<Cfg> create(Cfg pConfig, vk::BufferUsageFlags pBufferUsage, vk::MemoryPropertyFlags pMemoryProperties, size_t pNumBuffers)
	{
		cgb::buffer<Cfg> b;
		for (size_t i = 0; i < pNumBuffers; ++i) {
			auto bufferSize = pConfig.total_size();

			// Create (possibly multiple) buffer(s):
			auto bufferCreateInfo = vk::BufferCreateInfo()
				.setSize(static_cast<vk::DeviceSize>(bufferSize))
				.setUsage(pBufferUsage)
				// Always grant exclusive ownership to the queue.
				.setSharingMode(vk::SharingMode::eExclusive)
				// The flags parameter is used to configure sparse buffer memory, which is not relevant right now. We'll leave it at the default value of 0. [2]
				.setFlags(vk::BufferCreateFlags()); 

			// Create the buffer on the logical device
			auto vkBuffer = context().logical_device().createBuffer(bufferCreateInfo);

			// The buffer has been created, but it doesn't actually have any memory assigned to it yet. 
			// The first step of allocating memory for the buffer is to query its memory requirements [2]
			auto memRequirements = context().logical_device().getBufferMemoryRequirements(vkBuffer);

			auto allocInfo = vk::MemoryAllocateInfo()
				.setAllocationSize(memRequirements.size)
				.setMemoryTypeIndex(context().find_memory_type_index(
					memRequirements.memoryTypeBits, 
					pMemoryProperties));

			// Allocate the memory for the buffer:
			auto vkMemory = context().logical_device().allocateMemory(allocInfo);

			b.mInstances.push_back(std::make_tuple(bufferSize, pBufferUsage, vkBuffer, pMemoryProperties, vkMemory, pConfig));
		}
		return b;
	}

	/**	Return value of a create-function which creates one or multiple buffers.
	 *	These buffers can potentially also require some semaphores which you have to use in order to wait
	 *	for data transfer completion. You'll always get some semaphores back if you allocate memory on the 
	 */
	template <typename Cfg>
	struct buffer_and_semaphores
	{
		cgb::buffer<Cfg> mBuffer;
		std::vector<cgb::semaphore> mSemaphores;
	};

	template <typename Cfg>
	buffer_and_semaphores<Cfg> create(Cfg pConfig, cgb::memory_location pMemoryLocation, cgb::buffer_usage pUsage, const void* pData, std::optional<cgb::buffer_usage_details> pBufferUsageDetails = std::nullopt)
	{
		// Determine the number of buffers to allocate:
		uint32_t numBuffers = 1;
		switch (pUsage)
		{
		case cgb::buffer_usage::dynamic_for_render:
			if (!pBufferUsageDetails.has_value()) {
				// Default to the main window's data
				pBufferUsageDetails = cgb::buffer_usage_details::from_main_window();
			}
			numBuffers = pBufferUsageDetails->mNumBuffers;
			break;
		case cgb::buffer_usage::dynamic_for_update:
			if (!pBufferUsageDetails.has_value()) {
				// Default to a ping-pong buffer
				pBufferUsageDetails = cgb::buffer_usage_details::from_num_buffers(2);
			}
			numBuffers = pBufferUsageDetails->mNumBuffers;
			break;
		default:
			break;
		}

		auto bufferSize = pConfig.total_size();

		// We've got two major branches here: 
		// 1) Memory will stay on the host and there will be no dedicated memory on the device
		// 2) Memory will be transfered to the device. (Only in this case, we'll need to create semaphores.)
		if (pMemoryLocation < cgb::memory_location::device) {
			// HOST-ONLY Memory
			auto memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible;
			if (cgb::memory_location::host_coherent == pMemoryLocation) {
				memoryFlags |= vk::MemoryPropertyFlagBits::eHostCoherent;
			}
			if (cgb::memory_location::host_cached == pMemoryLocation) {
				memoryFlags |= vk::MemoryPropertyFlagBits::eHostCached;
			}

			auto buffers = cgb::create(cgb::host_buffer{ bufferSize }, vk::BufferUsageFlags(), memoryFlags, numBuffers);

			// Set the data:
			if (cgb::memory_location::host_coherent == pMemoryLocation) {
				// Copy data to coherently mapped host memory
				for (uint32_t i = 0; i < num_instances; ++i) {
					auto memory = buffer.memory_handle_at(i);
					void* mapped = context().logical_device().mapMemory(memory, 0, bufferSize);
					memcpy(mapped, pData, copySize);
					context().logical_device().unmapMemory(memory);
				}
			}
			if (cgb::memory_location::host_cached == pMemoryLocation) {
				// Copy data to non-coherently, cached mapped host memory
				for (uint32_t i = 0; i < num_instances; ++i) {
					auto memory = buffer.memory_handle_at(i);
					void* mapped = context().logical_device().mapMemory(memory, 0, bufferSize);
					memcpy(mapped, pData, copySize);
					context().logical_device().unmapMemory(memory); // TODO: FLUSH oder so??
				}
			}
		}
		else { 
			// Memory ON THE DEVICE

		}
		/** Write once and never modify again */
		static_only,
			/** Read and write constantly, but only have one instance. */
			dynamic_singleton,
			/** Dynamic buffer with as many copies as there are swap chain images */
			dynamic_for_render,
			/** Dynamic buffer with a configurable number of copies */
			dynamic_for_update

			/** Buffer's memory will be on the host and in "host coherent" mode */
			host_coherent,
			/** Buffer's memory will be on the host and in "host cached" mode */
			host_cached,
			/** Buffer's memory is on the GPU */
			device,
			/** Buffer's memory is on the GPU with "protected mode" set */
			device_protected


		auto bufferSize = pConfig.total_size();
		

		// TODO: Create a staging buffer, fill it (maybe with flush?)
		// Create another device buffer
		// Create the staging buffer contents into device buffer

		return result;
	}
	

	struct descriptor_pool
	{
		descriptor_pool() noexcept;
		descriptor_pool(const vk::DescriptorPool&);
		descriptor_pool(const descriptor_pool&) = delete;
		descriptor_pool(descriptor_pool&&) noexcept;
		descriptor_pool& operator=(const descriptor_pool&) = delete;
		descriptor_pool& operator=(descriptor_pool&&) noexcept;
		~descriptor_pool();

		vk::DescriptorPool mDescriptorPool;
	};

	struct descriptor_set
	{
		descriptor_set() noexcept;
		descriptor_set(const vk::DescriptorSet&);
		descriptor_set(const descriptor_set&) = delete;
		descriptor_set(descriptor_set&&) noexcept;
		descriptor_set& operator=(const descriptor_set&) = delete;
		descriptor_set& operator=(descriptor_set&&) noexcept;

		vk::DescriptorSet mDescriptorSet;
	};

	extern vk::ImageMemoryBarrier create_image_barrier(vk::Image pImage, vk::Format pFormat, vk::AccessFlags pSrcAccessMask, vk::AccessFlags pDstAccessMask, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, std::optional<vk::ImageSubresourceRange> pSubresourceRange = std::nullopt);

	struct image
	{
		image() noexcept;
		image(const vk::ImageCreateInfo&, const vk::Image&, const vk::DeviceMemory&) noexcept;
		image(const image&) = delete;
		image(image&&) noexcept;
		image& operator=(const image&) = delete;
		image& operator=(image&&) noexcept;
		~image();

		static image create2D(int width, int height, 
							  vk::Format format = vk::Format::eR8G8B8A8Unorm, 
							  vk::ImageTiling tiling = vk::ImageTiling::eOptimal, 
							  vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
							  vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::ImageMemoryBarrier create_barrier(vk::AccessFlags pSrcAccessMask, vk::AccessFlags pDstAccessMask, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, std::optional<vk::ImageSubresourceRange> pSubresourceRange = std::nullopt) const;

		vk::ImageCreateInfo mInfo;
		vk::Image mImage;
		vk::DeviceMemory mMemory;
	};

	extern void transition_image_layout(const image& pImage, vk::Format pFormat, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout);

	extern void copy_buffer_to_image(const buffer& pSrcBuffer, const image& pDstImage);

	struct command_buffer
	{
		void begin_recording();
		void end_recording();
		void begin_render_pass(const vk::RenderPass& pRenderPass, const vk::Framebuffer& pFramebuffer, const vk::Offset2D& pOffset, const vk::Extent2D& pExtent);
		void set_image_barrier(const vk::ImageMemoryBarrier& pBarrierInfo);
		void copy_image(const image& pSource, const vk::Image& pDestination);
		void end_render_pass();

		vk::CommandBufferBeginInfo mBeginInfo;
		vk::CommandBuffer mCommandBuffer;
	};

	struct image_view
	{
		image_view() noexcept;
		image_view(const vk::ImageViewCreateInfo& pInfo, const vk::ImageView& pImageView, const std::shared_ptr<image>& pImage);
		image_view(const image_view&) = delete;
		image_view(image_view&&) noexcept;
		image_view& operator=(const image_view&) = delete;
		image_view& operator=(image_view&&) noexcept;
		~image_view();

		static image_view create(const std::shared_ptr<image>& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags);

		vk::ImageViewCreateInfo mInfo;
		vk::ImageView mImageView;
		std::shared_ptr<image> mImage;
	};

	struct sampler
	{
		sampler() noexcept;
		sampler(const vk::Sampler& pSampler);
		sampler(const sampler&) = delete;
		sampler(sampler&&) noexcept;
		sampler& operator=(const sampler&) = delete;
		sampler& operator=(sampler&&) noexcept;
		~sampler();

		static sampler create();

		vk::Sampler mSampler;
	};

	struct descriptor_set_layout
	{
		descriptor_set_layout() noexcept;
		descriptor_set_layout(const vk::DescriptorSetLayout& pDescriptorSetLayout);
		descriptor_set_layout(const descriptor_set_layout&) = delete;
		descriptor_set_layout(descriptor_set_layout&&) noexcept;
		descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;
		descriptor_set_layout& operator=(descriptor_set_layout&&) noexcept;
		~descriptor_set_layout();

		static descriptor_set_layout create(const vk::DescriptorSetLayoutCreateInfo& pCreateInfo);

		vk::DescriptorSetLayout mDescriptorSetLayout;
	};


	struct acceleration_structure_handle
	{
		uint64_t mHandle;
	};

	struct acceleration_structure
	{
		acceleration_structure() noexcept;
		acceleration_structure(const vk::AccelerationStructureInfoNV& pAccStructureInfo, const vk::AccelerationStructureNV& pAccStructure, const acceleration_structure_handle& pHandle, const vk::MemoryPropertyFlags& pMemoryProperties, const vk::DeviceMemory& pMemory);
		acceleration_structure(const buffer&) = delete;
		acceleration_structure(acceleration_structure&&) noexcept;
		acceleration_structure& operator=(const acceleration_structure&) = delete;
		acceleration_structure& operator=(acceleration_structure&&) noexcept;
		~acceleration_structure();

		static acceleration_structure create_top_level(uint32_t pInstanceCount);
		static acceleration_structure create_bottom_level(const std::vector<vk::GeometryNV>& pGeometries);
		static acceleration_structure create(vk::AccelerationStructureTypeNV pType, const std::vector<vk::GeometryNV>& pGeometries, uint32_t pInstanceCount);
		
		size_t get_scratch_buffer_size();

		vk::AccelerationStructureInfoNV mAccStructureInfo; // TODO: This is potentially dangerous! The structure stores the pGeometries pointer which might have become invalid. What to do?
		vk::AccelerationStructureNV mAccStructure;
		acceleration_structure_handle mHandle;
		vk::MemoryPropertyFlags mMemoryProperties;
		vk::DeviceMemory mMemory;
	};

	struct shader_binding_table : public buffer
	{
		shader_binding_table() noexcept;
		shader_binding_table(size_t, const vk::BufferUsageFlags&, const vk::Buffer&, const vk::MemoryPropertyFlags&, const vk::DeviceMemory&) noexcept;
		shader_binding_table(const sampler&) = delete;
		shader_binding_table(shader_binding_table&&) noexcept;
		shader_binding_table& operator=(const shader_binding_table&) = delete;
		shader_binding_table& operator=(shader_binding_table&&) noexcept;
		~shader_binding_table();

		static shader_binding_table create(const pipeline& pRtPipeline);
	};


	enum struct device_queue_selection_strategy
	{
		prefer_separate_queues,
		prefer_everything_on_single_queue,
	};

	// Forward declare:
	struct queue_submit_proxy;

	/** Represents a device queue, storing the queue itself, 
	 *	the queue family's index, and the queue's index.
	*/
	struct device_queue
	{
		/** Contains all the prepared queues which will be passed to logical device creation. */
		static std::deque<device_queue> sPreparedQueues;

		/** Prepare another queue and eventually add it to `sPreparedQueues`. */
		static device_queue* prepare(
			vk::QueueFlags pFlagsRequired,
			device_queue_selection_strategy pSelectionStrategy,
			std::optional<vk::SurfaceKHR> pSupportForSurface);

		/** Create a new queue on the logical device. */
		static device_queue create(uint32_t pQueueFamilyIndex, uint32_t pQueueIndex);
		/** Create a new queue on the logical device. */
		static device_queue create(const device_queue& pPreparedQueue);

		/** Gets the queue family index of this queue */
		uint32_t family_index() const { return mQueueFamilyIndex; }
		/** Gets queue index (inside the queue family) of this queue. */
		uint32_t queue_index() const { return mQueueIndex; }

		uint32_t mQueueFamilyIndex;
		uint32_t mQueueIndex;
		float mPriority;
		vk::Queue mQueue;
	};

	struct queue_submit_proxy
	{
		queue_submit_proxy() = default;
		queue_submit_proxy(const queue_submit_proxy&) = delete;
		queue_submit_proxy(queue_submit_proxy&&) = delete;
		queue_submit_proxy& operator=(const queue_submit_proxy&) = delete;
		queue_submit_proxy& operator=(queue_submit_proxy&&) = delete;

		device_queue& mQueue;
		vk::SubmitInfo mSubmitInfo;
		std::vector<command_buffer> mCommandBuffers;
		std::vector<semaphore> mWaitSemaphores;
		std::vector<semaphore> mSignalSemaphores;
	};

}
