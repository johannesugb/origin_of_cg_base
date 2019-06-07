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

	/** Specifies for a buffer, where the buffer's memory will be located */
	enum struct memory_location
	{
		/** Buffer's memory will be on the host and in "host coherent" mode */
		host_coherent,
		/** Buffer's memory will be on the host and in "host cached" mode */
		host_cached,
		/** Buffer's memory is on the GPU */
		device,
		/** Buffer's memory is on the GPU with "protected mode" set */
		device_protected
	};

	/** Specifies how a buffer is going to be used. The "dynamic" versions 
	 *	usually imply that there will be many instances of the buffer;
	 *	e.g., more specifically, one buffer for each swap chain image, if 
	 *	      the buffer_usage is set to `dynamic_for_render`
	 */
	enum struct buffer_usage
	{
		/** Write once and never modify again */
		static_only,
		/** Read and write constantly, but only have one instance. */
		dynamic_singleton,
		/** Dynamic buffer with as many copies as there are swap chain images */
		dynamic_for_render,
		/** Dynamic buffer with a configurable number of copies */
		dynamic_for_update
	};

	/** Additional information for some cgb::buffer_usage settings */
	struct buffer_usage_details
	{
		static buffer_usage_details from_swap_chain(window* pWindow);
		static buffer_usage_details from_main_window();
		static buffer_usage_details from_num_buffers(uint32_t pNumBuffers);

		uint32_t mNumBuffers;
	};

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

	// Just a stub for the great create-functions which there are to come
	template <typename I, typename O>
	O create();

	/** Represents a Vulkan buffer along with its assigned memory, holds the 
	 *	native handle and takes care about lifetime management of the native handles.
	 */
	template <typename Cfg>
	struct buffer
	{
		buffer() noexcept = default;
		buffer(const buffer&) = delete;
		buffer(buffer&&) noexcept = default;
		buffer& operator=(const buffer&) = delete;
		buffer& operator=(buffer&&) noexcept = default;
		~buffer() {
			for (auto& inst : mInstances) {
				if (std::get<vk::Buffer>(inst)) {
					context().logical_device().destroyBuffer(std::get<vk::Buffer>(inst));
					std::get<vk::Buffer>(inst) = nullptr;
				}
				if (std::get<vk::Memory>(inst)) {
					context().logical_device().freeMemory(std::get<vk::Memory>(inst));
					std::get<vk::Memory>(inst) = nullptr;
				}
			}
		}

		auto size_at(size_t index) const				{ return std::get<size_t>(mInstances[index]); }
		auto usage_flags_at(size_t index) const			{ return std::get<vk::BufferUsageFlags>(mInstances[index]); }
		auto buffer_at(size_t index) const				{ return std::get<vk::Buffer>(mInstances[index]); }
		auto memory_properties_at(size_t index) const	{ return std::get<vk::MemoryPropertyFlags>(mInstances[index]); }
		auto memory_at(size_t index) const				{ return std::get<vk::DeviceMemory>(mInstances[index]); }
		auto config_at(size_t index) const				{ return std::get<Cfg>(mInstances[index]); }
		auto size() const								{ return size_at(0); }
		auto usage_flags() const						{ return usage_flags_at(0); }
		auto buffer() const								{ return buffer_at(0); }
		auto memory_properties() const					{ return memory_properties_at(0); }
		auto memory() const								{ return memory_at(0); }
		auto config() const								{ return config_at(0); }

		static buffer create(size_t pBufferSize, vk::BufferUsageFlags pUsageFlags, vk::MemoryPropertyFlags pMemoryProperties);
		void fill_host_coherent_memory(const void* pData, std::optional<size_t> pSize = std::nullopt);

		// mSize, mBufferFlags, mBuffer, mMemoryProperties, mMemory, config-type
		std::vector<std::tuple<size_t, vk::BufferUsageFlags, vk::Buffer, vk::MemoryPropertyFlags, vk::DeviceMemory, Cfg>> mInstances;
	};
	extern void copy(const buffer& pSource, const buffer& pDestination);
	
	/** This struct contains information for a staging buffer which is a temporary buffer 
	 *	usually used to transfer data from the CPU side to the GPU side.
	 */
	struct staging_buffer
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/**	This struct contains information for a buffer which is intended to be used as 
	 *	vertex buffer, i.e. vertex attributes provided to a shader.
	 */
	struct vertex_buffer
	{
		size_t size_one_element() const { return mSizeOneElement; }
		size_t num_elements() const { return mNumElements; }
		size_t total_size() const { return size_one_element() * num_elements(); }

		size_t mSizeOneElement;
		size_t mNumElements;
	};

	/**	This struct contains information for a buffer which is intended to be used as 
	*	index buffer.
	*/
	struct index_buffer
	{
		size_t size_one_element() const 
		{
			switch (mIndexType) {
			case vk::IndexType::eUint16:
				return sizeof(uint16_t);
			case vk::IndexType::eUint32:
				return sizeof(uint32_t);
			case vk::IndexType::eNoneNV:
				return 0;
			default:
				throw std::runtime_error("Unsupported vk::IndexType");
			}
		}
		size_t num_elements() const { return static_cast<size_t>(mIndexCount); }
		size_t total_size() const { return size_one_element() * num_elements(); }

		vk::IndexType mIndexType;
		uint32_t mIndexCount;
	};

	/** This struct contains information for a uniform buffer.
	*/
	struct uniform_buffer
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
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
	buffer_and_semaphores<Cfg> create(Cfg pConfig, cgb::memory_location pMemoryLocation, cgb::buffer_usage pUsage, const void* pData)
	{
		buffer_and_semaphores<Cfg> result;

		auto bufferSize = pConfig.total_size();
		

		cgb::create(cgb::staging_buffer{ bufferSize }, )
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

	struct fence
	{
		fence() noexcept;
		fence(const vk::FenceCreateInfo&, const vk::Fence&) noexcept;
		fence(const fence&) = delete;
		fence(fence&&) noexcept;
		fence& operator=(const fence&) = delete;
		fence& operator=(fence&&) noexcept;
		~fence();

		static fence create(const vk::FenceCreateInfo& pCreateInfo);

		vk::FenceCreateInfo mCreateInfo;
		vk::Fence mFence;
	};

	struct semaphore
	{
		semaphore() noexcept;
		semaphore(const vk::SemaphoreCreateInfo&, const vk::Semaphore&) noexcept;
		semaphore(const semaphore&) = delete;
		semaphore(semaphore&&) noexcept;
		semaphore& operator=(const semaphore&) = delete;
		semaphore& operator=(semaphore&&) noexcept;
		virtual ~semaphore();

		static semaphore create(const vk::SemaphoreCreateInfo& pCreateInfo);

		vk::SemaphoreCreateInfo mCreateInfo;
		vk::Semaphore mSemaphore;

		// --- Some advanced features of a semaphore object ---

		/** A custom deleter function called upon destruction of this semaphore */
		std::optional<std::function<void()>> mCustomDeleter;

		/** An optional dependant semaphore. This means: The dependant
		 *	semaphore can be assumed to be finished when this semaphore
		 *	has finished.
		 *	The point here is that some internal function might wait on it,
		 *	that shall be somewhat opaque to the user in some cases.
		 *	The dependant semaphore child object ensures that the semaphore
		 *	does not get destructed prematurely.
		 */
		std::optional<semaphore> mDependantSemaphore;
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
