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

	shader_handle shader_handle::create_from_binary_code(const std::vector<char>& code)
	{
		auto createInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(code.size())
			.setPCode(reinterpret_cast<const uint32_t*>(code.data()));
		return { context().mLogicalDevice.createShaderModule(createInfo) };
	}

	shader_handle::shader_handle() noexcept 
		: mShaderModule{}
	{ }

	shader_handle::shader_handle(const vk::ShaderModule& shaderModule) noexcept
		: mShaderModule{ shaderModule }
	{ }

	shader_handle::shader_handle(shader_handle&& other) noexcept
		: mShaderModule{ std::move(other.mShaderModule) }
	{
		other.mShaderModule = nullptr;
	}

	shader_handle& shader_handle::operator=(shader_handle&& other) noexcept
	{
		mShaderModule = std::move(other.mShaderModule);

		other.mShaderModule = nullptr;
		return *this;
	}

	shader_handle::~shader_handle()
	{
		if (mShaderModule) {
			context().mLogicalDevice.destroyShaderModule(mShaderModule);
			mShaderModule = nullptr;
		}
	}

	vk::ShaderStageFlagBits convert(shader_type p)
	{
		switch (p) {
		case cgb::shader_type::vertex:
			return vk::ShaderStageFlagBits::eVertex;
		case cgb::shader_type::tessellation_control:
			return vk::ShaderStageFlagBits::eTessellationControl;
		case cgb::shader_type::tessellation_evaluation:
			return vk::ShaderStageFlagBits::eTessellationEvaluation;
		case cgb::shader_type::geometry:
			return vk::ShaderStageFlagBits::eGeometry;
		case cgb::shader_type::fragment:
			return vk::ShaderStageFlagBits::eFragment;
		case cgb::shader_type::compute:
			return vk::ShaderStageFlagBits::eCompute;
		case cgb::shader_type::ray_generation:
			return vk::ShaderStageFlagBits::eRaygenNV;
		case cgb::shader_type::any_hit:
			return vk::ShaderStageFlagBits::eAnyHitNV;
		case cgb::shader_type::closest_hit:
			return vk::ShaderStageFlagBits::eClosestHitNV;
		case cgb::shader_type::miss:
			return vk::ShaderStageFlagBits::eMissNV;
		case cgb::shader_type::intersection:
			return vk::ShaderStageFlagBits::eIntersectionNV;
		case cgb::shader_type::callable:
			return vk::ShaderStageFlagBits::eCallableNV;
		case cgb::shader_type::task:
			return vk::ShaderStageFlagBits::eTaskNV;
		case cgb::shader_type::mesh:
			return vk::ShaderStageFlagBits::eMeshNV;
		default:
			throw std::runtime_error("Invalid shader_type");
		}
	}

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

	command_pool::command_pool() noexcept
		: mQueueFamilyIndex{ 0 }
		, mCreateInfo{}
		, mCommandPool{ nullptr }
	{ }

	command_pool::command_pool(uint32_t pQueueFamilyIndex, const vk::CommandPoolCreateInfo& pCreateInfo, const vk::CommandPool& pPool) noexcept
		: mQueueFamilyIndex{ pQueueFamilyIndex }
		, mCreateInfo{ pCreateInfo }
		, mCommandPool{ pPool }
	{ }

	command_pool::command_pool(command_pool&& other) noexcept
		: mQueueFamilyIndex{ std::move(other.mQueueFamilyIndex) }
		, mCreateInfo{ std::move(other.mCreateInfo) }
		, mCommandPool{ std::move(other.mCommandPool) }
	{
		other.mQueueFamilyIndex = 0;
		other.mCreateInfo = {};
		other.mCommandPool = nullptr;
	}

	command_pool& command_pool::operator=(command_pool&& other) noexcept
	{
		mQueueFamilyIndex = std::move(other.mQueueFamilyIndex);
		mCreateInfo = std::move(other.mCreateInfo);
		mCommandPool = std::move(other.mCommandPool);
		other.mQueueFamilyIndex = 0u;
		other.mCreateInfo = {};
		other.mCommandPool = nullptr;
		return *this;
	}

	command_pool::~command_pool()
	{
		if (mCommandPool) {
			context().logical_device().destroyCommandPool(mCommandPool);
			mCommandPool = nullptr;
		}
	}

	command_pool command_pool::create(uint32_t pQueueFamilyIndex, const vk::CommandPoolCreateInfo& pCreateInfo)
	{
		return command_pool{
			pQueueFamilyIndex,
			pCreateInfo,
			context().logical_device().createCommandPool(pCreateInfo)
		};
	}

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
		return &sPreparedQueues.emplace_back(device_queue{
			familyIndex, 
			queueIndex,
			0.5f, // default priority of 0.5
			nullptr
		});
	}

	device_queue device_queue::create(uint32_t pQueueFamilyIndex, uint32_t pQueueIndex)
	{
		return device_queue{
			pQueueFamilyIndex, 
			pQueueIndex,
			0.5f, // default priority of 0.5f
			context().logical_device().getQueue(pQueueFamilyIndex, pQueueIndex)
		};
	}

	device_queue device_queue::create(const device_queue& pPreparedQueue)
	{
		return device_queue{
			pPreparedQueue.family_index(),
			pPreparedQueue.queue_index(),
			pPreparedQueue.mPriority,
			context().logical_device().getQueue(pPreparedQueue.family_index(), pPreparedQueue.queue_index())
		};
	}


	void command_buffer::begin_recording()
	{
		mCommandBuffer.begin(mBeginInfo);
	}

	void command_buffer::end_recording()
	{
		mCommandBuffer.end();
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
		
		mCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		// 2nd parameter: how the drawing commands within the render pass will be provided. It can have one of two values [7]:
		//  - VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
		//  - VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed from secondary command buffers.
	}

	void command_buffer::set_image_barrier(const vk::ImageMemoryBarrier& pBarrierInfo)
	{
		mCommandBuffer.pipelineBarrier(
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
		mCommandBuffer.copyImage(pSource.mImage, vk::ImageLayout::eTransferSrcOptimal, pDestination, vk::ImageLayout::eTransferDstOptimal, { copyInfo });
	}

	void command_buffer::end_render_pass()
	{
		mCommandBuffer.endRenderPass();
	}


	buffer buffer::create(size_t pBufferSize, vk::BufferUsageFlags pUsageFlags, vk::MemoryPropertyFlags pMemoryProperties)
	{
		auto bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(static_cast<vk::DeviceSize>(pBufferSize))
			.setUsage(pUsageFlags)
			.setFlags(vk::BufferCreateFlags()); // The flags parameter is used to configure sparse buffer memory, which is not relevant right now. We'll leave it at the default value of 0. [2]
		
		if ((pUsageFlags & vk::BufferUsageFlagBits::eTransferSrc) == vk::BufferUsageFlagBits::eTransferSrc) {
			// It is intended for transfer => let the context set the appropriate sharing mode
			context().set_sharing_mode_for_transfer(bufferCreateInfo);
		}
		else {
			// If it is not intended for transfer => just set to exclusive!
			bufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
		}

		auto vkBuffer = context().logical_device().createBuffer(bufferCreateInfo);

		// The buffer has been created, but it doesn't actually have any memory assigned to it yet. 
		// The first step of allocating memory for the buffer is to query its memory requirements [2]
		auto memRequirements = context().logical_device().getBufferMemoryRequirements(vkBuffer);

		auto allocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(context().find_memory_type_index(
				memRequirements.memoryTypeBits, 
				pMemoryProperties));

		auto vkMemory = context().logical_device().allocateMemory(allocInfo);

		// If memory allocation was successful, then we can now associate this memory with the buffer
		cgb::context().logical_device().bindBufferMemory(vkBuffer, vkMemory, 0);

		return buffer{
			pBufferSize,
			pUsageFlags,
			vkBuffer,
			pMemoryProperties,
			vkMemory
		};
	}

	void buffer::fill_host_coherent_memory(const void* pData, std::optional<size_t> pSize)
	{
		assert((mMemoryProperties & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)) == (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		vk::DeviceSize copySize = pSize ? *pSize : mSize;
		void* mapped = context().logical_device().mapMemory(mMemory, 0, copySize);
		memcpy(mapped, pData, copySize);
		context().logical_device().unmapMemory(mMemory);
	}

	void copy(const buffer& pSource, const buffer& pDestination)
	{
		auto commandBuffer = context().create_command_buffers_for_transfer(1);

		// Immediately start recording the command buffer:
		commandBuffer[0].begin_recording();

		auto copyRegion = vk::BufferCopy{}
			.setSrcOffset(0u)
			.setDstOffset(0u)
			.setSize(static_cast<vk::DeviceSize>(pSource.mSize));
		commandBuffer[0].mCommandBuffer.copyBuffer(pSource.mBuffer, pDestination.mBuffer, { copyRegion });

		// That's all
		commandBuffer[0].end_recording();

		auto submitInfo = vk::SubmitInfo{}
			.setCommandBufferCount(1u)
			.setPCommandBuffers(&commandBuffer[0].mCommandBuffer);
		cgb::context().transfer_queue().submit({ submitInfo }, nullptr); // not using fence... TODO: maybe use fence!
		cgb::context().transfer_queue().waitIdle();
	}

	vertex_buffer::vertex_buffer() noexcept
		: buffer()
		, mVertexCount(0)
	{ }

	vertex_buffer::vertex_buffer(vertex_buffer&& other) noexcept
		: buffer(std::move(other))
		, mVertexCount(std::move(other.mVertexCount))
	{
		other.mVertexCount = 0;
	}

	vertex_buffer& vertex_buffer::operator=(vertex_buffer&& other) noexcept
	{
		buffer::operator=(std::move(other));
		mVertexCount = std::move(other.mVertexCount);
		other.mVertexCount = 0;
		return *this;
	}

	vertex_buffer vertex_buffer::create(size_t pVertexDataSize, size_t pVertexCount, vk::BufferUsageFlags pAdditionalBufferUsageFlags, vk::MemoryPropertyFlags pMemoryProperties)
	{
		auto b = buffer::create(pVertexDataSize * pVertexCount, vk::BufferUsageFlagBits::eVertexBuffer | pAdditionalBufferUsageFlags, pMemoryProperties);

		auto vertexBuffer = vertex_buffer();
		static_cast<cgb::buffer&>(vertexBuffer) = std::move(b);
		vertexBuffer.mVertexCount = static_cast<uint32_t>(pVertexCount);
		return vertexBuffer;
	}


	index_buffer::index_buffer() noexcept
		: buffer()
		, mIndexType()
		, mIndexCount(0u)
	{ }

	index_buffer::index_buffer(index_buffer&& other) noexcept
		: buffer(std::move(other))
		, mIndexType(std::move(other.mIndexType))
		, mIndexCount(std::move(other.mIndexCount))
	{ 
		other.mIndexType = vk::IndexType();
		other.mIndexCount = 0u;
	}

	index_buffer& index_buffer::operator=(index_buffer&& other) noexcept
	{
		buffer::operator=(std::move(other));
		mIndexType = std::move(other.mIndexType);
		mIndexCount = std::move(other.mIndexCount);
		other.mIndexType = vk::IndexType();
		other.mIndexCount = 0u;
		return *this;
	}

	index_buffer index_buffer::create(vk::IndexType pIndexType, size_t pIndexCount, vk::BufferUsageFlags pAdditionalBufferUsageFlags, vk::MemoryPropertyFlags pMemoryProperties)
	{
		size_t elSize = 0;
		switch (pIndexType) {
		case vk::IndexType::eUint16:
			elSize = sizeof(uint16_t);
			break;
		case vk::IndexType::eUint32:
			elSize = sizeof(uint32_t);
			break;
		case vk::IndexType::eNoneNV:
			elSize = 0;
			break;
		default:
			throw std::runtime_error("Can't handle that vk::IndexType");
		}
		auto b = buffer::create(elSize * pIndexCount, vk::BufferUsageFlagBits::eIndexBuffer | pAdditionalBufferUsageFlags, pMemoryProperties);

		auto indexBuffer = index_buffer();
		static_cast<cgb::buffer&>(indexBuffer) = std::move(b);
		indexBuffer.mIndexType = pIndexType;
		indexBuffer.mIndexCount = static_cast<uint32_t>(pIndexCount);
		return indexBuffer;
	}

	uniform_buffer::uniform_buffer() noexcept
		: buffer()
	{ }

	uniform_buffer::uniform_buffer(uniform_buffer&& other) noexcept
		: buffer(std::move(other))
	{ }

	uniform_buffer& uniform_buffer::operator=(uniform_buffer&& other) noexcept
	{
		buffer::operator=(std::move(other));
		return *this;
	}

	uniform_buffer uniform_buffer::create(size_t pBufferSize, vk::BufferUsageFlags pAdditionalBufferUsageFlags, vk::MemoryPropertyFlags pMemoryProperties)
	{
		auto b = buffer::create(pBufferSize, vk::BufferUsageFlagBits::eUniformBuffer | pAdditionalBufferUsageFlags, pMemoryProperties);

		auto uniformBuffer = uniform_buffer();
		static_cast<cgb::buffer&>(uniformBuffer) = std::move(b);
		return uniformBuffer;
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
		auto commandBuffer = context().create_command_buffers_for_graphics(1, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		// Immediately start recording the command buffer:
		commandBuffer[0].begin_recording();

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
		commandBuffer[0].mCommandBuffer.pipelineBarrier(
			sourceStageFlags,
			destinationStageFlags,
			vk::DependencyFlags(), // The third parameter is either 0 or VK_DEPENDENCY_BY_REGION_BIT. The latter turns the barrier into a per-region condition. That means that the implementation is allowed to already begin reading from the parts of a resource that were written so far, for example. [3]
			{},
			{},
			{ barrier });

		// That's all
		commandBuffer[0].end_recording();
		
		auto submitInfo = vk::SubmitInfo()
			.setCommandBufferCount(1u)
			.setPCommandBuffers(&commandBuffer[0].mCommandBuffer);
		cgb::context().graphics_queue().submit({ submitInfo }, nullptr); // not using fence... TODO: maybe use fence!
		cgb::context().graphics_queue().waitIdle();
	}
	
	void copy_buffer_to_image(const buffer& pSrcBuffer, const image& pDstImage)
	{
		auto commandBuffer = context().create_command_buffers_for_transfer(1);

		// Immediately start recording the command buffer:
		commandBuffer[0].begin_recording();

		auto copyRegion = vk::BufferImageCopy()
			.setBufferOffset(0)
			// The bufferRowLength and bufferImageHeight fields specify how the pixels are laid out in memory. For example, you could have some padding 
			// bytes between rows of the image. Specifying 0 for both indicates that the pixels are simply tightly packed like they are in our case. [3]
			.setBufferRowLength(0)
			.setBufferImageHeight(0)
			.setImageSubresource(vk::ImageSubresourceLayers()
								 .setAspectMask(vk::ImageAspectFlagBits::eColor)
								 .setMipLevel(0u)
								 .setBaseArrayLayer(0u)
								 .setLayerCount(1u))
			.setImageOffset({ 0u, 0u, 0u })
			.setImageExtent(pDstImage.mInfo.extent);

		commandBuffer[0].mCommandBuffer.copyBufferToImage(
			pSrcBuffer.mBuffer, 
			pDstImage.mImage, 
			vk::ImageLayout::eTransferDstOptimal,
			{ copyRegion });

		// That's all
		commandBuffer[0].end_recording();

		auto submitInfo = vk::SubmitInfo()
			.setCommandBufferCount(1u)
			.setPCommandBuffers(&commandBuffer[0].mCommandBuffer);
		cgb::context().transfer_queue().submit({ submitInfo }, nullptr); // not using fence... TODO: maybe use fence!
		cgb::context().transfer_queue().waitIdle();
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

	acceleration_structure::acceleration_structure(const vk::AccelerationStructureInfoNV& pAccStructureInfo, const vk::AccelerationStructureNV& pAccStructure, const acceleration_structure_handle& pHandle, const vk::MemoryPropertyFlags& pMemoryProperties, const vk::DeviceMemory& pMemory)
		: mAccStructureInfo(pAccStructureInfo)
		, mAccStructure(pAccStructure)
		, mHandle(pHandle)
		, mMemoryProperties(pMemoryProperties)
		, mMemory(pMemory)
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

		return acceleration_structure(accInfo, accStructure, handle, memPropertyFlags, deviceMemory);
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
		: buffer()
	{ }

	shader_binding_table::shader_binding_table(size_t pSize, const vk::BufferUsageFlags& pBufferFlags, const vk::Buffer& pBuffer, const vk::MemoryPropertyFlags& pMemoryProperties, const vk::DeviceMemory& pMemory) noexcept
		: buffer(pSize, pBufferFlags, pBuffer, pMemoryProperties, pMemory)
	{ }

	shader_binding_table::shader_binding_table(shader_binding_table&& other) noexcept
		: buffer(std::move(other))
	{ }

	shader_binding_table& shader_binding_table::operator=(shader_binding_table&& other) noexcept
	{ 
		buffer::operator=(std::move(other));
		return *this;
	}

	shader_binding_table::~shader_binding_table()
	{ }

	shader_binding_table shader_binding_table::create(const pipeline& pRtPipeline)
	{
		auto numGroups = 5u; // TODO: store groups in `pipeline` (or rather in `ray_tracing_pipeline : pipeline`) and then, read from pRtPipeline
		auto rtProps = context().get_ray_tracing_properties();
		auto shaderBindingTableSize = rtProps.shaderGroupHandleSize * numGroups;

		auto b = buffer::create(shaderBindingTableSize,
								vk::BufferUsageFlagBits::eTransferSrc,
								vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		void* mapped = context().logical_device().mapMemory(b.mMemory, 0, b.mSize);
		// Transfer something into the buffer's memory...
		context().logical_device().getRayTracingShaderGroupHandlesNV(pRtPipeline.mPipeline, 0, numGroups, b.mSize, mapped, context().dynamic_dispatch());
		context().logical_device().unmapMemory(b.mMemory);
		
		auto sbt = shader_binding_table();
		static_cast<buffer&>(sbt) = std::move(b);
		return sbt;
	}

	fence::fence() noexcept
		: mCreateInfo{}
		, mFence{nullptr}
	{ }

	fence::fence(const vk::FenceCreateInfo& pCreateInfo, const vk::Fence& pFence) noexcept
		: mCreateInfo{pCreateInfo}
		, mFence{pFence}
	{ }

	fence::fence(fence&& other) noexcept
		: mCreateInfo{ std::move(other.mCreateInfo) }
		, mFence{ std::move(other.mFence) }
	{ 
		other.mCreateInfo = {};
		other.mFence = nullptr;
	}

	fence& fence::operator=(fence&& other) noexcept
	{ 
		mCreateInfo = std::move(other.mCreateInfo);
		mFence = std::move(other.mFence);
		other.mCreateInfo = {};
		other.mFence = nullptr;
		return *this;
	}

	fence::~fence()
	{ 
		if (mFence) {
			context().logical_device().destroyFence(mFence);
			mFence = nullptr;
		}
	}

	fence fence::create(const vk::FenceCreateInfo& pCreateInfo)
	{ 
		return fence{
			pCreateInfo,
			context().logical_device().createFence(pCreateInfo)
		};
	}


	semaphore::semaphore() noexcept
		: mCreateInfo{}
		, mSemaphore{nullptr}
	{ }

	semaphore::semaphore(const vk::SemaphoreCreateInfo& pCreateInfo, const vk::Semaphore& pSemaphore) noexcept
		: mCreateInfo{pCreateInfo}
		, mSemaphore{pSemaphore}
	{ }

	semaphore::semaphore(semaphore&& other) noexcept
		: mCreateInfo{ std::move(other.mCreateInfo) }
		, mSemaphore{ std::move(other.mSemaphore) }
	{ 
		other.mCreateInfo = {};
		other.mSemaphore = nullptr;
	}

	semaphore& semaphore::operator=(semaphore&& other) noexcept
	{ 
		mCreateInfo = std::move(other.mCreateInfo);
		mSemaphore = std::move(other.mSemaphore);
		other.mCreateInfo = {};
		other.mSemaphore = nullptr;
		return *this;
	}

	semaphore::~semaphore()
	{ 
		if (mSemaphore) {
			context().logical_device().destroySemaphore(mSemaphore);
			mSemaphore = nullptr;
		}
	}

	semaphore semaphore::create(const vk::SemaphoreCreateInfo& pCreateInfo)
	{ 
		return semaphore{
			pCreateInfo,
			context().logical_device().createSemaphore(pCreateInfo)
		};
	}

	// [1] Vulkan Tutorial, Rendering and presentation, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
	// [2] Vulkan Tutorial, Vertex buffer creation, https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
	// [3] Vulkan Tutorial, Images, https://vulkan-tutorial.com/Texture_mapping/Images
	// [4] Vulkan Tutorial, Image view and sampler, https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler
}
