#include "context_vulkan_types.h"
#include <set>

namespace cgb
{
	
	image_format::image_format() noexcept
	{ }

	image_format::image_format(const vk::Format& pFormat) noexcept
		: mFormat{ pFormat }
	{ }

	image_format::image_format(const vk::SurfaceFormatKHR& pSrfFrmt) noexcept 
		: mFormat{ pSrfFrmt.format }
	{ }

	bool is_srgb_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> srgbFormats = {
			vk::Format::eR8Srgb,
			vk::Format::eR8G8Srgb,
			vk::Format::eR8G8B8Srgb,
			vk::Format::eB8G8R8Srgb,
			vk::Format::eR8G8B8A8Srgb,
			vk::Format::eB8G8R8A8Srgb,
			vk::Format::eA8B8G8R8SrgbPack32
		};
		auto it = std::find(std::begin(srgbFormats), std::end(srgbFormats), pImageFormat.mFormat);
		return it != srgbFormats.end();
	}

	bool is_uint8_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed formats are ignored => could/should be added in the future, maybe
		// TODO: sRGB-formats are assumed to be uint8-formats (not signed int8-formats) => is that true?
		static std::set<vk::Format> uint8Formats = {
			vk::Format::eR8Unorm,
			vk::Format::eR8Uscaled,
			vk::Format::eR8Uint,
			vk::Format::eR8Srgb,
			vk::Format::eR8G8Unorm,
			vk::Format::eR8G8Uscaled,
			vk::Format::eR8G8Uint,
			vk::Format::eR8G8Srgb,
			vk::Format::eR8G8B8Unorm,
			vk::Format::eR8G8B8Uscaled,
			vk::Format::eR8G8B8Uint,
			vk::Format::eR8G8B8Srgb,
			vk::Format::eB8G8R8Unorm,
			vk::Format::eB8G8R8Uscaled,
			vk::Format::eB8G8R8Uint,
			vk::Format::eB8G8R8Srgb,
			vk::Format::eR8G8B8A8Unorm,
			vk::Format::eR8G8B8A8Uscaled,
			vk::Format::eR8G8B8A8Uint,
			vk::Format::eR8G8B8A8Srgb,
			vk::Format::eB8G8R8A8Unorm,
			vk::Format::eB8G8R8A8Uscaled,
			vk::Format::eB8G8R8A8Uint,
			vk::Format::eB8G8R8A8Srgb,
			vk::Format::eA8B8G8R8UnormPack32,
			vk::Format::eA8B8G8R8UscaledPack32,
			vk::Format::eA8B8G8R8UintPack32,
			vk::Format::eA8B8G8R8SrgbPack32
		};
		auto it = std::find(std::begin(uint8Formats), std::end(uint8Formats), pImageFormat.mFormat);
		return it != uint8Formats.end();
	}

	bool is_int8_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> int8Formats = {
			vk::Format::eR8Snorm,
			vk::Format::eR8Sscaled,
			vk::Format::eR8Sint,
			vk::Format::eR8G8Snorm,
			vk::Format::eR8G8Sscaled,
			vk::Format::eR8G8Sint,
			vk::Format::eR8G8B8Snorm,
			vk::Format::eR8G8B8Sscaled,
			vk::Format::eR8G8B8Sint,
			vk::Format::eB8G8R8Snorm,
			vk::Format::eB8G8R8Sscaled,
			vk::Format::eB8G8R8Sint,
			vk::Format::eR8G8B8A8Snorm,
			vk::Format::eR8G8B8A8Sscaled,
			vk::Format::eR8G8B8A8Sint,
			vk::Format::eB8G8R8A8Snorm,
			vk::Format::eB8G8R8A8Sscaled,
			vk::Format::eB8G8R8A8Sint,
			vk::Format::eA8B8G8R8SnormPack32,
			vk::Format::eA8B8G8R8SscaledPack32,
			vk::Format::eA8B8G8R8SintPack32,
		};
		auto it = std::find(std::begin(int8Formats), std::end(int8Formats), pImageFormat.mFormat);
		return it != int8Formats.end();
	}

	bool is_uint16_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> uint16Formats = {
			vk::Format::eR16Unorm,
			vk::Format::eR16Uscaled,
			vk::Format::eR16Uint,
			vk::Format::eR16G16Unorm,
			vk::Format::eR16G16Uscaled,
			vk::Format::eR16G16Uint,
			vk::Format::eR16G16B16Unorm,
			vk::Format::eR16G16B16Uscaled,
			vk::Format::eR16G16B16Uint,
			vk::Format::eR16G16B16A16Unorm,
			vk::Format::eR16G16B16A16Uscaled,
			vk::Format::eR16G16B16A16Uint
		};
		auto it = std::find(std::begin(uint16Formats), std::end(uint16Formats), pImageFormat.mFormat);
		return it != uint16Formats.end();
	}

	bool is_int16_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> int16Formats = {
			vk::Format::eR16Snorm,
			vk::Format::eR16Sscaled,
			vk::Format::eR16Sint,
			vk::Format::eR16G16Snorm,
			vk::Format::eR16G16Sscaled,
			vk::Format::eR16G16Sint,
			vk::Format::eR16G16B16Snorm,
			vk::Format::eR16G16B16Sscaled,
			vk::Format::eR16G16B16Sint,
			vk::Format::eR16G16B16A16Snorm,
			vk::Format::eR16G16B16A16Sscaled,
			vk::Format::eR16G16B16A16Sint
		};
		auto it = std::find(std::begin(int16Formats), std::end(int16Formats), pImageFormat.mFormat);
		return it != int16Formats.end();
	}

	bool is_uint32_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> uint32Format = { 
			vk::Format::eR32Uint,
			vk::Format::eR32G32Uint,
			vk::Format::eR32G32B32Uint,
			vk::Format::eR32G32B32A32Uint
		};
		auto it = std::find(std::begin(uint32Format), std::end(uint32Format), pImageFormat.mFormat);
		return it != uint32Format.end();
	}

	bool is_int32_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> int32Format = {
			vk::Format::eR32Sint,
			vk::Format::eR32G32Sint,
			vk::Format::eR32G32B32Sint,
			vk::Format::eR32G32B32A32Sint
		};
		auto it = std::find(std::begin(int32Format), std::end(int32Format), pImageFormat.mFormat);
		return it != int32Format.end();
	}

	bool is_float16_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> float16Formats = {
			vk::Format::eR16Sfloat,
			vk::Format::eR16G16Sfloat,
			vk::Format::eR16G16B16Sfloat,
			vk::Format::eR16G16B16A16Sfloat
		};
		auto it = std::find(std::begin(float16Formats), std::end(float16Formats), pImageFormat.mFormat);
		return it != float16Formats.end();
	}

	bool is_float32_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> float32Formats = {
			vk::Format::eR32Sfloat,
			vk::Format::eR32G32Sfloat,
			vk::Format::eR32G32B32Sfloat,
			vk::Format::eR32G32B32A32Sfloat
		};
		auto it = std::find(std::begin(float32Formats), std::end(float32Formats), pImageFormat.mFormat);
		return it != float32Formats.end();
	}

	bool is_float64_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> float64Formats = {
			vk::Format::eR64Sfloat,
			vk::Format::eR64G64Sfloat,
			vk::Format::eR64G64B64Sfloat,
			vk::Format::eR64G64B64A64Sfloat
		};
		auto it = std::find(std::begin(float64Formats), std::end(float64Formats), pImageFormat.mFormat);
		return it != float64Formats.end();
	}

	bool is_rgb_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> rgbFormats = {
			vk::Format::eR5G6B5UnormPack16,
			vk::Format::eR8G8B8Unorm,
			vk::Format::eR8G8B8Snorm,
			vk::Format::eR8G8B8Uscaled,
			vk::Format::eR8G8B8Sscaled,
			vk::Format::eR8G8B8Uint,
			vk::Format::eR8G8B8Sint,
			vk::Format::eR8G8B8Srgb,
			vk::Format::eR16G16B16Unorm,
			vk::Format::eR16G16B16Snorm,
			vk::Format::eR16G16B16Uscaled,
			vk::Format::eR16G16B16Sscaled,
			vk::Format::eR16G16B16Uint,
			vk::Format::eR16G16B16Sint,
			vk::Format::eR16G16B16Sfloat,
			vk::Format::eR32G32B32Uint,
			vk::Format::eR32G32B32Sint,
			vk::Format::eR32G32B32Sfloat,
			vk::Format::eR64G64B64Uint,
			vk::Format::eR64G64B64Sint,
			vk::Format::eR64G64B64Sfloat,

		};
		auto it = std::find(std::begin(rgbFormats), std::end(rgbFormats), pImageFormat.mFormat);
		return it != rgbFormats.end();
	}

	bool is_rgba_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> rgbaFormats = {
			vk::Format::eR4G4B4A4UnormPack16,
			vk::Format::eR5G5B5A1UnormPack16,
			vk::Format::eR8G8B8A8Unorm,
			vk::Format::eR8G8B8A8Snorm,
			vk::Format::eR8G8B8A8Uscaled,
			vk::Format::eR8G8B8A8Sscaled,
			vk::Format::eR8G8B8A8Uint,
			vk::Format::eR8G8B8A8Sint,
			vk::Format::eR8G8B8A8Srgb,
			vk::Format::eR16G16B16A16Unorm,
			vk::Format::eR16G16B16A16Snorm,
			vk::Format::eR16G16B16A16Uscaled,
			vk::Format::eR16G16B16A16Sscaled,
			vk::Format::eR16G16B16A16Uint,
			vk::Format::eR16G16B16A16Sint,
			vk::Format::eR16G16B16A16Sfloat,
			vk::Format::eR32G32B32A32Uint,
			vk::Format::eR32G32B32A32Sint,
			vk::Format::eR32G32B32A32Sfloat,
			vk::Format::eR64G64B64A64Uint,
			vk::Format::eR64G64B64A64Sint,
			vk::Format::eR64G64B64A64Sfloat,
		};
		auto it = std::find(std::begin(rgbaFormats), std::end(rgbaFormats), pImageFormat.mFormat);
		return it != rgbaFormats.end();
	}

	bool is_argb_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> argbFormats = {
			vk::Format::eA1R5G5B5UnormPack16,
			vk::Format::eA2R10G10B10UnormPack32,
			vk::Format::eA2R10G10B10SnormPack32,
			vk::Format::eA2R10G10B10UscaledPack32,
			vk::Format::eA2R10G10B10SscaledPack32,
			vk::Format::eA2R10G10B10UintPack32,
			vk::Format::eA2R10G10B10SintPack32,
		};
		auto it = std::find(std::begin(argbFormats), std::end(argbFormats), pImageFormat.mFormat);
		return it != argbFormats.end();
	}

	bool is_bgr_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> bgrFormats = {
			vk::Format::eB5G6R5UnormPack16,
			vk::Format::eB8G8R8Unorm,
			vk::Format::eB8G8R8Snorm,
			vk::Format::eB8G8R8Uscaled,
			vk::Format::eB8G8R8Sscaled,
			vk::Format::eB8G8R8Uint,
			vk::Format::eB8G8R8Sint,
			vk::Format::eB8G8R8Srgb,
			vk::Format::eB10G11R11UfloatPack32,
		};
		auto it = std::find(std::begin(bgrFormats), std::end(bgrFormats), pImageFormat.mFormat);
		return it != bgrFormats.end();
	}

	bool is_bgra_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> bgraFormats = {
			vk::Format::eB4G4R4A4UnormPack16,
			vk::Format::eB5G5R5A1UnormPack16,
			vk::Format::eR8G8B8A8Unorm,
			vk::Format::eR8G8B8A8Snorm,
			vk::Format::eR8G8B8A8Uscaled,
			vk::Format::eR8G8B8A8Sscaled,
			vk::Format::eR8G8B8A8Uint,
			vk::Format::eR8G8B8A8Sint,
			vk::Format::eR8G8B8A8Srgb,
			vk::Format::eB8G8R8A8Unorm,
			vk::Format::eB8G8R8A8Snorm,
			vk::Format::eB8G8R8A8Uscaled,
			vk::Format::eB8G8R8A8Sscaled,
			vk::Format::eB8G8R8A8Uint,
			vk::Format::eB8G8R8A8Sint,
			vk::Format::eB8G8R8A8Srgb,
		};
		auto it = std::find(std::begin(bgraFormats), std::end(bgraFormats), pImageFormat.mFormat);
		return it != bgraFormats.end();
	}

	bool is_abgr_format(const image_format& pImageFormat)
	{
		// Note: Currently, the compressed sRGB-formats are ignored => could/should be added in the future, maybe
		static std::set<vk::Format> abgrFormats = {
			vk::Format::eA8B8G8R8UnormPack32,
			vk::Format::eA8B8G8R8SnormPack32,
			vk::Format::eA8B8G8R8UscaledPack32,
			vk::Format::eA8B8G8R8SscaledPack32,
			vk::Format::eA8B8G8R8UintPack32,
			vk::Format::eA8B8G8R8SintPack32,
			vk::Format::eA8B8G8R8SrgbPack32,
			vk::Format::eA2B10G10R10UnormPack32,
			vk::Format::eA2B10G10R10SnormPack32,
			vk::Format::eA2B10G10R10UscaledPack32,
			vk::Format::eA2B10G10R10SscaledPack32,
			vk::Format::eA2B10G10R10UintPack32,
			vk::Format::eA2B10G10R10SintPack32,
		};
		auto it = std::find(std::begin(abgrFormats), std::end(abgrFormats), pImageFormat.mFormat);
		return it != abgrFormats.end();
	}

	bool has_stencil_component(const image_format& pImageFormat)
	{
		static std::set<vk::Format> stencilFormats = {
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD24UnormS8Uint,
		};
		auto it = std::find(std::begin(stencilFormats), std::end(stencilFormats), pImageFormat.mFormat);
		return it != stencilFormats.end();
	}

	vk::IndexType convert_to_vk_index_type(size_t pSize)
	{
		if (pSize == sizeof(uint16_t)) {
			return vk::IndexType::eUint16;
		}
		if (pSize == sizeof(uint32_t)) {
			return vk::IndexType::eUint32;
		}
		LOG_ERROR(fmt::format("The given size[{}] does not correspond to a valid vk::IndexType", pSize));
		return vk::IndexType::eNoneNV;
	}

#pragma region command_buffer
	std::vector<command_buffer> command_buffer::create_many(uint32_t pCount, command_pool& pPool, vk::CommandBufferUsageFlags pUsageFlags)
	{
		auto bufferAllocInfo = vk::CommandBufferAllocateInfo()
			.setCommandPool(pPool.handle())
			.setLevel(vk::CommandBufferLevel::ePrimary) // Those, allocated from a pool, are primary command buffers; secondary command buffers can be allocated from command buffers.
			.setCommandBufferCount(pCount);

		auto tmp = context().logical_device().allocateCommandBuffersUnique(bufferAllocInfo);
		
		// Iterate over all the "raw"-Vk objects in `tmp` and...
		std::vector<command_buffer> buffers;
		buffers.reserve(pCount);
		std::transform(std::begin(tmp), std::end(tmp),
			std::back_inserter(buffers),
			// ...transform them into `cgb::command_buffer` objects:
			[usageFlags = pUsageFlags](auto& vkCb) {
				command_buffer result;
				result.mBeginInfo = vk::CommandBufferBeginInfo()
					.setFlags(usageFlags)
					.setPInheritanceInfo(nullptr);
				result.mCommandBuffer = std::move(vkCb);
				return result;
			});
		return buffers;
	}

	command_buffer command_buffer::create(command_pool& pPool, vk::CommandBufferUsageFlags pUsageFlags)
	{
		auto result = std::move(command_buffer::create_many(1, pPool, pUsageFlags)[0]);
		return result;
	}
#pragma endregion

#pragma region command_pool
	command_pool command_pool::create(uint32_t pQueueFamilyIndex, vk::CommandPoolCreateFlags pCreateFlags)
	{
		auto createInfo = vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(pQueueFamilyIndex)
			.setFlags(pCreateFlags); // Optional
		// Possible values for the flags [7]
		//  - VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
		//  - VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
		command_pool result;
		result.mQueueFamilyIndex = pQueueFamilyIndex;
		result.mCreateInfo = createInfo;
		result.mCommandPool = context().logical_device().createCommandPoolUnique(createInfo);
		return result;
	}

	std::vector<command_buffer> command_pool::get_command_buffers(uint32_t pCount, vk::CommandBufferUsageFlags pUsageFlags)
	{
		return command_buffer::create_many(pCount, *this, pUsageFlags);
	}

	command_buffer command_pool::get_command_buffer(vk::CommandBufferUsageFlags pUsageFlags)
	{
		return command_buffer::create(*this, pUsageFlags);
	}
#pragma endregion

#pragma region device_queue
	std::deque<device_queue> device_queue::sPreparedQueues;

	device_queue* device_queue::prepare(
		vk::QueueFlags pFlagsRequired,
		device_queue_selection_strategy pSelectionStrategy,
		std::optional<vk::SurfaceKHR> pSupportForSurface)
	{
		auto families = context().find_best_queue_family_for(pFlagsRequired, pSelectionStrategy, pSupportForSurface);
		if (families.size() == 0) {
			throw std::runtime_error("Couldn't find queue families satisfying the given criteria.");
		}

		// Default to the first ones, each
		uint32_t familyIndex = std::get<0>(families[0]);
		uint32_t queueIndex = 0;

		for (auto& family : families) {
			for (uint32_t qi = 0; qi < std::get<1>(family).queueCount; ++qi) {

				auto alreadyInUse = std::find_if(
					std::begin(sPreparedQueues), 
					std::end(sPreparedQueues), 
					[familyIndexInQuestion = std::get<0>(family), queueIndexInQuestion = qi](const auto& pq) {
					return pq.family_index() == familyIndexInQuestion
						&& pq.queue_index() == queueIndexInQuestion;
				});

				// Pay attention to different selection strategies:
				switch (pSelectionStrategy)
				{
				case cgb::device_queue_selection_strategy::prefer_separate_queues:
					if (sPreparedQueues.end() == alreadyInUse) {
						// didn't find combination, that's good
						familyIndex = std::get<0>(family);
						queueIndex = qi;
						goto found_indices;
					}
					break;
				case cgb::device_queue_selection_strategy::prefer_everything_on_single_queue:
					if (sPreparedQueues.end() != alreadyInUse) {
						// find combination, that's good in this case
						familyIndex = std::get<0>(family);
						queueIndex = qi;
						goto found_indices;
					}
					break;
				}
			}
		}

	found_indices:
		auto& prepd_queue = sPreparedQueues.emplace_back();
		prepd_queue.mQueueFamilyIndex = familyIndex;
		prepd_queue.mQueueIndex = queueIndex;
		prepd_queue.mPriority = 0.5f; // default priority of 0.5
		prepd_queue.mQueue = nullptr;
		return &prepd_queue;
	}

	device_queue device_queue::create(uint32_t pQueueFamilyIndex, uint32_t pQueueIndex)
	{
		device_queue result;
		result.mQueueFamilyIndex = pQueueFamilyIndex;
		result.mQueueIndex = pQueueIndex;
		result.mPriority = 0.5f; // default priority of 0.5f
		result.mQueue = context().logical_device().getQueue(result.mQueueFamilyIndex, result.mQueueIndex);
		return result;
	}

	device_queue device_queue::create(const device_queue& pPreparedQueue)
	{
		device_queue result;
		result.mQueueFamilyIndex = pPreparedQueue.family_index();
		result.mQueueIndex = pPreparedQueue.queue_index();
		result.mPriority = pPreparedQueue.mPriority; // default priority of 0.5f
		result.mQueue = context().logical_device().getQueue(result.mQueueFamilyIndex, result.mQueueIndex);
		return result;
	}

	command_pool& device_queue::pool() const 
	{ 
		return context().get_command_pool_for_queue(*this); 
	}
#pragma endregion














	pipeline::pipeline() noexcept
		: mRenderPass{}
		, mPipelineLayout{}
		, mPipeline{}
	{ }

	pipeline::pipeline(const vk::PipelineLayout& pPipelineLayout, const vk::Pipeline& pPipeline, vk::RenderPass pRenderPass) noexcept
		: mRenderPass{ pRenderPass }
		, mPipelineLayout{ pPipelineLayout }
		, mPipeline{ pPipeline }
	{ }

	pipeline::pipeline(pipeline&& other) noexcept
		: mRenderPass{ std::move(other.mRenderPass) }
		, mPipelineLayout{ std::move(other.mPipelineLayout) }
		, mPipeline{ std::move(other.mPipeline) }
	{ 
		other.mRenderPass = nullptr;
		other.mPipelineLayout = nullptr;
		other.mPipeline = nullptr;
	}
	
	pipeline& pipeline::operator=(pipeline&& other) noexcept 
	{ 
		mRenderPass = std::move(other.mRenderPass);
		mPipelineLayout = std::move(other.mPipelineLayout);
		mPipeline = std::move(other.mPipeline);

		other.mRenderPass = nullptr;
		other.mPipelineLayout = nullptr;
		other.mPipeline = nullptr;
		return *this;
	}

	pipeline::~pipeline()
	{
		if (mRenderPass) {
			context().mLogicalDevice.destroyRenderPass(mRenderPass);
			mRenderPass = nullptr;
		}
		if (mPipelineLayout) {
			context().mLogicalDevice.destroyPipelineLayout(mPipelineLayout);
			mPipelineLayout = nullptr;
		}
		if (mPipeline) {
			context().mLogicalDevice.destroyPipeline(mPipeline);
			mPipeline = nullptr;
		}
	}


	framebuffer::framebuffer() noexcept
		: mFramebuffer()
	{ }

	framebuffer::framebuffer(const vk::Framebuffer& pFramebuffer) noexcept
		: mFramebuffer(pFramebuffer)
	{ }

	framebuffer::framebuffer(framebuffer&& other) noexcept
		: mFramebuffer(std::move(other.mFramebuffer))
	{
		other.mFramebuffer = nullptr;
	}

	framebuffer& framebuffer::operator=(framebuffer&& other) noexcept
	{
		mFramebuffer = std::move(other.mFramebuffer);
		other.mFramebuffer = nullptr;
		return *this;
	}

	framebuffer::~framebuffer()
	{
		if (mFramebuffer) {
			context().mLogicalDevice.destroyFramebuffer(mFramebuffer);
			mFramebuffer = nullptr;
		}
	}


	


	void command_buffer::begin_recording()
	{
		mCommandBuffer->begin(mBeginInfo);
	}

	void command_buffer::end_recording()
	{
		mCommandBuffer->end();
	}

	void command_buffer::begin_render_pass(const vk::RenderPass& pRenderPass, const vk::Framebuffer& pFramebuffer, const vk::Offset2D& pOffset, const vk::Extent2D& pExtent)
	{
		std::array clearValues = {
			vk::ClearValue(vk::ClearColorValue{ make_array<float>( 0.5f, 0.0f, 0.5f, 1.0f ) }),
			vk::ClearValue(vk::ClearDepthStencilValue{ 1.0f, 0 })
		};
		// TODO: how to determine the number of attachments => and the number of clear-values? omg...

		auto renderPassBeginInfo = vk::RenderPassBeginInfo()
			.setRenderPass(pRenderPass)
			.setFramebuffer(pFramebuffer)
			.setRenderArea(vk::Rect2D()
						   .setOffset(pOffset)
						   .setExtent(pExtent))
			.setClearValueCount(static_cast<uint32_t>(clearValues.size()))
			.setPClearValues(clearValues.data());
		
		mCommandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		// 2nd parameter: how the drawing commands within the render pass will be provided. It can have one of two values [7]:
		//  - VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
		//  - VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed from secondary command buffers.
	}

	void command_buffer::set_image_barrier(const vk::ImageMemoryBarrier& pBarrierInfo)
	{
		mCommandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::DependencyFlags(),
			{}, {}, { pBarrierInfo });
	}

	void command_buffer::copy_image(const image& pSource, const vk::Image& pDestination)
	{ // TODO: fix this hack after the RTX-VO!
		auto fullImageOffset = vk::Offset3D(0, 0, 0);
		auto fullImageExtent = pSource.mInfo.extent;
		auto halfImageOffset = vk::Offset3D(0, 0, 0); //vk::Offset3D(pSource.mInfo.extent.width / 2, 0, 0);
		auto halfImageExtent = vk::Extent3D(pSource.mInfo.extent.width / 2, pSource.mInfo.extent.height, pSource.mInfo.extent.depth);
		auto offset = halfImageOffset;
		auto extent = halfImageExtent;

		auto copyInfo = vk::ImageCopy()
			.setSrcSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0u, 0u, 1u))
			.setSrcOffset(offset)
			.setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0u, 0u, 1u))
			.setDstOffset(offset)
			.setExtent(extent);
		mCommandBuffer->copyImage(pSource.mImage, vk::ImageLayout::eTransferSrcOptimal, pDestination, vk::ImageLayout::eTransferDstOptimal, { copyInfo });
	}

	void command_buffer::end_render_pass()
	{
		mCommandBuffer->endRenderPass();
	}

	descriptor_pool::descriptor_pool() noexcept
		: mDescriptorPool()
	{ }

	descriptor_pool::descriptor_pool(const vk::DescriptorPool& pDescriptorPool)
		: mDescriptorPool(pDescriptorPool)
	{ }

	descriptor_pool::descriptor_pool(descriptor_pool&& other) noexcept
		: mDescriptorPool(std::move(other.mDescriptorPool))
	{
		other.mDescriptorPool = nullptr;
	}

	descriptor_pool& descriptor_pool::operator=(descriptor_pool&& other) noexcept
	{
		mDescriptorPool = std::move(other.mDescriptorPool);
		other.mDescriptorPool = nullptr;
		return *this;
	}

	descriptor_pool::~descriptor_pool()
	{
		if (mDescriptorPool) {
			context().logical_device().destroyDescriptorPool(mDescriptorPool);
			mDescriptorPool = nullptr;
		}
	}

	descriptor_set::descriptor_set() noexcept 
		: mDescriptorSet() 
	{ }

	descriptor_set::descriptor_set(const vk::DescriptorSet& pDescriptorSet)
		: mDescriptorSet(pDescriptorSet)
	{ }

	descriptor_set::descriptor_set(descriptor_set&& other) noexcept
		: mDescriptorSet(std::move(other.mDescriptorSet))
	{ 
		other.mDescriptorSet = nullptr;
	}

	descriptor_set& descriptor_set::operator=(descriptor_set&& other) noexcept
	{ 
		mDescriptorSet = std::move(other.mDescriptorSet);
		other.mDescriptorSet = nullptr;
		return *this;
	}

	image::image() noexcept
		: mInfo(), mImage(), mMemory()
	{ }

	image::image(const vk::ImageCreateInfo& pInfo, const vk::Image& pImage, const vk::DeviceMemory& pMemory) noexcept
		: mInfo(pInfo), mImage(pImage), mMemory(pMemory)
	{ }

	image::image(image&& other) noexcept
		: mInfo(std::move(other.mInfo))
		, mImage(std::move(other.mImage))
		, mMemory(std::move(other.mMemory))
	{ 
		other.mInfo = vk::ImageCreateInfo();
		other.mImage = nullptr;
		other.mMemory = nullptr;
	}

	image& image::operator=(image&& other) noexcept
	{
		mInfo = std::move(other.mInfo);
		mImage = std::move(other.mImage);
		mMemory = std::move(other.mMemory);
		other.mInfo = vk::ImageCreateInfo();
		other.mImage = nullptr;
		other.mMemory = nullptr;
		return *this;
	}

	image::~image()
	{
		if (mImage) {
			context().logical_device().destroyImage(mImage);
			mImage = nullptr;
		}
		if (mMemory) {
			context().logical_device().freeMemory(mMemory);
			mMemory = nullptr;
		}
	}

	image image::create2D(int width, int height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
	{
		auto imageInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setExtent(vk::Extent3D(static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1u))
			.setMipLevels(1u)
			.setArrayLayers(1u)
			.setFormat(format)
			.setTiling(tiling)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setUsage(usage)
			.setSharingMode(vk::SharingMode::eExclusive) // The image will only be used by one queue family: the one that supports graphics (and therefore also) transfer operations. [3]
			.setSamples(vk::SampleCountFlagBits::e1)
			.setFlags(vk::ImageCreateFlags()); // Optional
		auto vkImage = context().logical_device().createImage(imageInfo);

		auto memRequirements = context().logical_device().getImageMemoryRequirements(vkImage);

		auto allocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(context().find_memory_type_index(memRequirements.memoryTypeBits, properties));

		auto vkMemory = context().logical_device().allocateMemory(allocInfo);

		// bind together:
		context().logical_device().bindImageMemory(vkImage, vkMemory, 0);
		return image(imageInfo, vkImage, vkMemory);
	}

	vk::ImageMemoryBarrier create_image_barrier(vk::Image pImage, vk::Format pFormat, vk::AccessFlags pSrcAccessMask, vk::AccessFlags pDstAccessMask, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, std::optional<vk::ImageSubresourceRange> pSubresourceRange)
	{
		if (!pSubresourceRange) {
			vk::ImageAspectFlags aspectMask;
			if (pNewLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
				aspectMask = vk::ImageAspectFlagBits::eDepth;
				if (has_stencil_component(cgb::image_format(pFormat))) {
					aspectMask |= vk::ImageAspectFlagBits::eStencil;
				}
			}
			else {
				aspectMask = vk::ImageAspectFlagBits::eColor;
			}

			pSubresourceRange = vk::ImageSubresourceRange()
				.setAspectMask(aspectMask)
				.setBaseMipLevel(0u)
				.setLevelCount(1u)
				.setBaseArrayLayer(0u)
				.setLayerCount(1u);
		}

		return vk::ImageMemoryBarrier()
			.setOldLayout(pOldLayout)
			.setNewLayout(pNewLayout)
			// If you are using the barrier to transfer queue family ownership, then these two fields should be the indices of the queue 
			// families.They must be set to VK_QUEUE_FAMILY_IGNORED if you don't want to do this (not the default value!). [3]
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setImage(pImage)
			.setSubresourceRange(*pSubresourceRange)
			.setSrcAccessMask(pSrcAccessMask)
			.setDstAccessMask(pDstAccessMask);
	}

	vk::ImageMemoryBarrier image::create_barrier(vk::AccessFlags pSrcAccessMask, vk::AccessFlags pDstAccessMask, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, std::optional<vk::ImageSubresourceRange> pSubresourceRange) const
	{
		return create_image_barrier(mImage, mInfo.format, pSrcAccessMask, pDstAccessMask, pOldLayout, pNewLayout, pSubresourceRange);
	}

	void transition_image_layout(const image& pImage, vk::Format pFormat, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout)
	{
		//auto commandBuffer = context().create_command_buffers_for_graphics(1, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		auto commandBuffer = context().graphics_queue().pool().get_command_buffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		// Immediately start recording the command buffer:
		commandBuffer.begin_recording();

		vk::AccessFlags sourceAccessMask, destinationAccessMask;
		vk::PipelineStageFlags sourceStageFlags, destinationStageFlags;

		// There are two transitions we need to handle [3]:
		//  - Undefined --> transfer destination : transfer writes that don't need to wait on anything
		//  - Transfer destination --> shader reading : shader reads should wait on transfer writes, specifically the shader reads in the fragment shader, because that's where we're going to use the texture
		if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout == vk::ImageLayout::eTransferDstOptimal) {
			sourceAccessMask = vk::AccessFlags();
			destinationAccessMask = vk::AccessFlagBits::eTransferWrite;
			sourceStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStageFlags = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			sourceAccessMask = vk::AccessFlagBits::eTransferWrite;
			destinationAccessMask = vk::AccessFlagBits::eShaderRead;
			sourceStageFlags = vk::PipelineStageFlagBits::eTransfer;
			destinationStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			sourceAccessMask = vk::AccessFlags();
			destinationAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			sourceStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStageFlags = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else {
			throw std::invalid_argument("unsupported layout transition");
		}


		// One of the most common ways to perform layout transitions is using an image memory barrier. A pipeline barrier like that 
		// is generally used to synchronize access to resources, like ensuring that a write to a buffer completes before reading from 
		// it, but it can also be used to transition image layouts and transfer queue family ownership when VK_SHARING_MODE_EXCLUSIVE 
		// is used.There is an equivalent buffer memory barrier to do this for buffers. [3]
		auto barrier = pImage.create_barrier(sourceAccessMask, destinationAccessMask, pOldLayout, pNewLayout);

		// The pipeline stages that you are allowed to specify before and after the barrier depend on how you use the resource before and 
		// after the barrier.The allowed values are listed in this table of the specification.For example, if you're going to read from a 
		// uniform after the barrier, you would specify a usage of VK_ACCESS_UNIFORM_READ_BIT and the earliest shader that will read from 
		// the uniform as pipeline stage, for example VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT. It would not make sense to specify a non-shader 
		// pipeline stage for this type of usage and the validation layers will warn you when you specify a pipeline stage that does not 
		// match the type of usage. [3]
		commandBuffer.handle().pipelineBarrier(
			sourceStageFlags,
			destinationStageFlags,
			vk::DependencyFlags(), // The third parameter is either 0 or VK_DEPENDENCY_BY_REGION_BIT. The latter turns the barrier into a per-region condition. That means that the implementation is allowed to already begin reading from the parts of a resource that were written so far, for example. [3]
			{},
			{},
			{ barrier });

		// That's all
		commandBuffer.end_recording();
		
		auto submitInfo = vk::SubmitInfo()
			.setCommandBufferCount(1u)
			.setPCommandBuffers(commandBuffer.handle_addr());
		cgb::context().graphics_queue().handle().submit({ submitInfo }, nullptr); // not using fence... TODO: maybe use fence!
		cgb::context().graphics_queue().handle().waitIdle();
	}
	
	
	image_view::image_view() noexcept
		: mInfo()
		, mImageView()
		, mImage()
	{ }
	
	image_view::image_view(const vk::ImageViewCreateInfo& pInfo, const vk::ImageView& pImageView, const std::shared_ptr<image>& pImage)
		: mInfo(pInfo)
		, mImageView(pImageView)
		, mImage(pImage)
	{ }
	
	image_view::image_view(image_view&& other) noexcept
		: mInfo(std::move(other.mInfo))
		, mImageView(std::move(other.mImageView))
		, mImage(std::move(other.mImage))
	{ 
		other.mInfo = vk::ImageViewCreateInfo();
		other.mImageView = nullptr;
		other.mImage.reset(); // TODO: should not be neccessary, because it has already been moved from, right?
	}

	image_view& image_view::operator=(image_view&& other) noexcept
	{ 
		mInfo = std::move(other.mInfo);
		mImageView = std::move(other.mImageView);
		mImage = std::move(other.mImage);
		other.mInfo = vk::ImageViewCreateInfo();
		other.mImageView = nullptr;
		other.mImage.reset(); // TODO: should not be neccessary, because it has already been moved from, right?
		return *this;
	}

	image_view::~image_view()
	{ 
		if (mImageView) {
			context().logical_device().destroyImageView(mImageView);
			mImageView = nullptr;
		}
	}

	image_view image_view::create(const std::shared_ptr<image>& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags)
	{ 
		auto viewInfo = vk::ImageViewCreateInfo()
			.setImage(pImage->mImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(pFormat)
			.setSubresourceRange(vk::ImageSubresourceRange()
								 .setAspectMask(pAspectFlags)
								 .setBaseMipLevel(0u)
								 .setLevelCount(1u)
								 .setBaseArrayLayer(0u)
								 .setLayerCount(1u));
		return image_view(viewInfo, context().logical_device().createImageView(viewInfo), pImage);
	}

	sampler::sampler() noexcept
		: mSampler()
	{ }

	sampler::sampler(const vk::Sampler& pSampler)
		: mSampler(pSampler)
	{ }

	sampler::sampler(sampler&& other) noexcept
		: mSampler(std::move(other.mSampler))
	{ 
		other.mSampler = nullptr;
	}

	sampler& sampler::operator=(sampler&& other) noexcept
	{
		mSampler = std::move(other.mSampler);
		other.mSampler = nullptr;
		return *this;
	}

	sampler::~sampler()
	{
		if (mSampler) {
			context().logical_device().destroySampler(mSampler);
			mSampler = nullptr;
		}
	}

	sampler sampler::create()
	{
		auto samplerInfo = vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setAnisotropyEnable(VK_TRUE)
			.setMaxAnisotropy(16.0f)
			.setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
			// The unnormalizedCoordinates field specifies which coordinate system you want to use to address texels in an image. 
			// If this field is VK_TRUE, then you can simply use coordinates within the [0, texWidth) and [0, texHeight) range.
			// If it is VK_FALSE, then the texels are addressed using the [0, 1) range on all axes. Real-world applications almost 
			// always use normalized coordinates, because then it's possible to use textures of varying resolutions with the exact 
			// same coordinates. [4]
			.setUnnormalizedCoordinates(VK_FALSE)
			// If a comparison function is enabled, then texels will first be compared to a value, and the result of that comparison 
			// is used in filtering operations. This is mainly used for percentage-closer filtering on shadow maps. [4]
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setMipLodBias(0.0f)
			.setMinLod(0.0f)
			.setMaxLod(0.0f);
		return sampler(context().logical_device().createSampler(samplerInfo));
	}

	descriptor_set_layout::descriptor_set_layout() noexcept
		: mDescriptorSetLayout()
	{ }

	descriptor_set_layout::descriptor_set_layout(const vk::DescriptorSetLayout& pDescriptorSetLayout)
		: mDescriptorSetLayout(pDescriptorSetLayout)
	{ }

	descriptor_set_layout::descriptor_set_layout(descriptor_set_layout&& other) noexcept
		: mDescriptorSetLayout(std::move(other.mDescriptorSetLayout))
	{
		other.mDescriptorSetLayout = vk::DescriptorSetLayout();
	}

	descriptor_set_layout& descriptor_set_layout::operator=(descriptor_set_layout&& other) noexcept
	{
		mDescriptorSetLayout = std::move(other.mDescriptorSetLayout);
		other.mDescriptorSetLayout = vk::DescriptorSetLayout();
		return *this;
	}

	descriptor_set_layout::~descriptor_set_layout()
	{
		if (mDescriptorSetLayout) {
			context().logical_device().destroyDescriptorSetLayout(mDescriptorSetLayout);
			mDescriptorSetLayout = nullptr;
		}
	}

	descriptor_set_layout descriptor_set_layout::create(const vk::DescriptorSetLayoutCreateInfo& pCreateInfo)
	{
		return descriptor_set_layout(context().logical_device().createDescriptorSetLayout(pCreateInfo));
	}

	acceleration_structure::acceleration_structure() noexcept
		: mAccStructureInfo()
		, mAccStructure(nullptr)
		, mHandle()
		, mMemoryProperties()
		, mMemory(nullptr)
	{ }

	acceleration_structure::acceleration_structure(acceleration_structure&& other) noexcept
		: mAccStructureInfo(std::move(other.mAccStructureInfo))
		, mAccStructure(std::move(other.mAccStructure))
		, mHandle(std::move(other.mHandle))
		, mMemoryProperties(std::move(other.mMemoryProperties))
		, mMemory(std::move(other.mMemory))
	{ 
		other.mAccStructureInfo = vk::AccelerationStructureInfoNV();
		other.mAccStructure = nullptr;
		other.mHandle = acceleration_structure_handle();
		other.mMemoryProperties = vk::MemoryPropertyFlags();
		other.mMemory = nullptr;
	}

	acceleration_structure& acceleration_structure::operator=(acceleration_structure&& other) noexcept
	{ 
		mAccStructureInfo = std::move(other.mAccStructureInfo);
		mAccStructure = std::move(other.mAccStructure);
		mHandle = std::move(other.mHandle);
		mMemoryProperties = std::move(other.mMemoryProperties);
		mMemory = std::move(other.mMemory);
		other.mAccStructureInfo = vk::AccelerationStructureInfoNV();
		other.mAccStructure = nullptr;
		other.mHandle = acceleration_structure_handle();
		other.mMemoryProperties = vk::MemoryPropertyFlags();
		other.mMemory = nullptr;
		return *this;
	}

	acceleration_structure::~acceleration_structure()
	{
		if (mAccStructure) {
			context().logical_device().destroyAccelerationStructureNV(mAccStructure, nullptr, cgb::context().dynamic_dispatch());
			mAccStructure = nullptr;
		}
		if (mMemory) {
			context().logical_device().freeMemory(mMemory);
			mMemory = nullptr;
		}
	}

	acceleration_structure acceleration_structure::create_top_level(uint32_t pInstanceCount)
	{
		return acceleration_structure::create(vk::AccelerationStructureTypeNV::eTopLevel, {}, pInstanceCount);
	}

	acceleration_structure acceleration_structure::create_bottom_level(const std::vector<vk::GeometryNV>& pGeometries)
	{
		return acceleration_structure::create(vk::AccelerationStructureTypeNV::eBottomLevel, pGeometries, 0);
	}

	acceleration_structure acceleration_structure::create(vk::AccelerationStructureTypeNV pType, const std::vector<vk::GeometryNV>& pGeometries, uint32_t pInstanceCount)
	{
		assert(pType == vk::AccelerationStructureTypeNV::eBottomLevel && pGeometries.size() > 0 || pInstanceCount > 0);
		// If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV then geometryCount must be 0
		// If type is VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV then instanceCount must be 0

		auto accInfo = vk::AccelerationStructureInfoNV()
			.setType(pType)
			.setFlags(vk::BuildAccelerationStructureFlagsNV())
			.setInstanceCount(pType == vk::AccelerationStructureTypeNV::eBottomLevel ? 0 : pInstanceCount)
			.setGeometryCount(pType == vk::AccelerationStructureTypeNV::eTopLevel ? 0 : static_cast<uint32_t>(pGeometries.size()))
			.setPGeometries(pType == vk::AccelerationStructureTypeNV::eTopLevel ? nullptr : pGeometries.data());

		auto createInfo = vk::AccelerationStructureCreateInfoNV()
			.setCompactedSize(0)
			.setInfo(accInfo);
		auto accStructure = context().logical_device().createAccelerationStructureNV(createInfo, nullptr, cgb::context().dynamic_dispatch());

		auto accStructMemInfo = vk::AccelerationStructureMemoryRequirementsInfoNV()
			.setAccelerationStructure(accStructure)
			.setType(vk::AccelerationStructureMemoryRequirementsTypeNV::eObject);
		auto memRequirements = context().logical_device().getAccelerationStructureMemoryRequirementsNV(accStructMemInfo, cgb::context().dynamic_dispatch());

		auto memPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

		auto allocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.memoryRequirements.size)
			.setMemoryTypeIndex(context().find_memory_type_index(
				memRequirements.memoryRequirements.memoryTypeBits,
				memPropertyFlags));
		auto deviceMemory = context().logical_device().allocateMemory(allocInfo);

		// bind memory to acceleration structure
		auto bindInfo = vk::BindAccelerationStructureMemoryInfoNV()
			.setAccelerationStructure(accStructure)
			.setMemory(deviceMemory)
			.setMemoryOffset(0)
			.setDeviceIndexCount(0)
			.setPDeviceIndices(nullptr);
		context().logical_device().bindAccelerationStructureMemoryNV({ bindInfo }, cgb::context().dynamic_dispatch());

		acceleration_structure_handle handle;
		context().logical_device().getAccelerationStructureHandleNV(accStructure, sizeof(handle.mHandle), &handle.mHandle, cgb::context().dynamic_dispatch());

		//return acceleration_structure(accInfo, accStructure, handle, memPropertyFlags, deviceMemory);
		return acceleration_structure{};
	}

	size_t acceleration_structure::get_scratch_buffer_size()
	{
		auto memReqInfo = vk::AccelerationStructureMemoryRequirementsInfoNV()
			.setAccelerationStructure(mAccStructure)
			.setType(vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch);

		auto memRequirements = context().logical_device().getAccelerationStructureMemoryRequirementsNV(memReqInfo, cgb::context().dynamic_dispatch());
		return static_cast<size_t>(memRequirements.memoryRequirements.size);
	}

	shader_binding_table::shader_binding_table() noexcept
	{ }

	shader_binding_table::shader_binding_table(size_t pSize, const vk::BufferUsageFlags& pBufferFlags, const vk::Buffer& pBuffer, const vk::MemoryPropertyFlags& pMemoryProperties, const vk::DeviceMemory& pMemory) noexcept
	{ }

	shader_binding_table::shader_binding_table(shader_binding_table&& other) noexcept
	{ }

	shader_binding_table& shader_binding_table::operator=(shader_binding_table&& other) noexcept
	{ 
		return *this;
	}

	shader_binding_table::~shader_binding_table()
	{ }

	shader_binding_table shader_binding_table::create(const pipeline& pRtPipeline)
	{
		auto numGroups = 5u; // TODO: store groups in `pipeline` (or rather in `ray_tracing_pipeline : pipeline`) and then, read from pRtPipeline
		auto rtProps = context().get_ray_tracing_properties();
		auto shaderBindingTableSize = rtProps.shaderGroupHandleSize * numGroups;

		// TODO: Use *new* buffer_t
		//auto b = buffer::create(shaderBindingTableSize,
		//						vk::BufferUsageFlagBits::eTransferSrc,
		//						vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		//void* mapped = context().logical_device().mapMemory(b.mMemory, 0, b.mSize);
		//// Transfer something into the buffer's memory...
		//context().logical_device().getRayTracingShaderGroupHandlesNV(pRtPipeline.mPipeline, 0, numGroups, b.mSize, mapped, context().dynamic_dispatch());
		//context().logical_device().unmapMemory(b.mMemory);
		
		auto sbt = shader_binding_table();
		//static_cast<buffer&>(sbt) = std::move(b);
		return sbt;
	}



	// [1] Vulkan Tutorial, Rendering and presentation, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
	// [2] Vulkan Tutorial, Vertex buffer creation, https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
	// [3] Vulkan Tutorial, Images, https://vulkan-tutorial.com/Texture_mapping/Images
	// [4] Vulkan Tutorial, Image view and sampler, https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler
}
