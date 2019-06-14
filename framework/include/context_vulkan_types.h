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

	// Forsward-declare the command pool
	class command_pool;

	/** A command buffer which has been created for a certain queue family */
	class command_buffer
	{
	public:
		void begin_recording();
		void end_recording();
		void begin_render_pass(const vk::RenderPass& pRenderPass, const vk::Framebuffer& pFramebuffer, const vk::Offset2D& pOffset, const vk::Extent2D& pExtent);
		void set_image_barrier(const vk::ImageMemoryBarrier& pBarrierInfo);
		void copy_image(const image& pSource, const vk::Image& pDestination);
		void end_render_pass();

		static std::vector<command_buffer> create_many(uint32_t pCount, command_pool& pPool, vk::CommandBufferUsageFlags pUsageFlags);
		static command_buffer create(command_pool& pPool, vk::CommandBufferUsageFlags pUsageFlags);

		vk::CommandBufferBeginInfo mBeginInfo;
		vk::UniqueCommandBuffer mCommandBuffer;
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
	class command_pool
	{
	public:
		command_pool() = default;
		command_pool(const command_pool&) = delete;
		command_pool(command_pool&&) = default;
		command_pool& operator=(const command_pool&) = delete;
		command_pool& operator=(command_pool&&) = default;
		~command_pool() = default;

		auto queue_family_index() const { return mQueueFamilyIndex; }
		const auto& create_info() const { return mCreateInfo; }
		const auto& handle() const { return mCommandPool.get(); }
		const auto* handle_addr() const { return &mCommandPool.get(); }
		
		static command_pool create(uint32_t pQueueFamilyIndex, vk::CommandPoolCreateFlags pCreateFlags = vk::CommandPoolCreateFlags());

		std::vector<command_buffer> get_command_buffers(uint32_t pCount, vk::CommandBufferUsageFlags pUsageFlags);
		command_buffer get_command_buffer(vk::CommandBufferUsageFlags pUsageFlags);

	private:
		uint32_t mQueueFamilyIndex;
		vk::CommandPoolCreateInfo mCreateInfo;
		vk::UniqueCommandPool mCommandPool;
	};

	/** Allows to configure whether to go for many or for a single queue. */
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
	class device_queue
	{
	public:
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
		auto family_index() const { return mQueueFamilyIndex; }
		/** Gets queue index (inside the queue family) of this queue. */
		auto queue_index() const { return mQueueIndex; }
		const auto& handle() const { return mQueue; }
		const auto* handle_addr() const { return &mQueue; }

		/** Gets a pool which is usable for this queue and the current thread. */
		auto& pool() const { return context().get_command_pool_for_queue(*this); }

	private:
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




}
