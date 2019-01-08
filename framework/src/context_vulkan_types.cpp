#include "context_vulkan_types.h"
#include <set>

namespace cgb
{
	image_format::image_format() noexcept
	{ }

	image_format::image_format(const vk::SurfaceFormatKHR& pSrfFrmt) noexcept 
		: mFormat{ pSrfFrmt.format }, mColorSpace{ pSrfFrmt.colorSpace }
	{ }

	image_format::image_format(const vk::Format& pFormat, const vk::ColorSpaceKHR& pColorSpace) noexcept
		: mFormat{ pFormat }, mColorSpace{ pColorSpace }
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

	shader_handle shader_handle::create_from_binary_code(const std::vector<char>& code)
	{
		auto createInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(code.size())
			.setPCode(reinterpret_cast<const uint32_t*>(code.data()));
		return { context().mLogicalDevice.createShaderModule(createInfo) };
	}

	shader_handle::shader_handle() noexcept 
		: mShaderModule()
	{ }

	shader_handle::shader_handle(const vk::ShaderModule& shaderModule) noexcept
		: mShaderModule(shaderModule)
	{ }

	shader_handle::shader_handle(shader_handle&& other) noexcept
		: mShaderModule(std::move(other.mShaderModule))
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
		: mRenderPass()
		, mPipelineLayout()
		, mPipeline()
	{ }

	pipeline::pipeline(const vk::RenderPass& pRenderPass, const vk::PipelineLayout& pPipelineLayout, const vk::Pipeline& pPipeline) noexcept
		: mRenderPass(pRenderPass)
		, mPipelineLayout(pPipelineLayout)
		, mPipeline(pPipeline)
	{ }

	pipeline::pipeline(pipeline&& other) noexcept
		: mRenderPass(std::move(other.mRenderPass))
		, mPipelineLayout(std::move(other.mPipelineLayout))
		, mPipeline(std::move(other.mPipeline))
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
		: mQueueFamilyIndex(0u)
		, mCommandPool()
	{ }

	command_pool::command_pool(uint32_t pQueueFamilyIndex, const vk::CommandPool& pCommandPool) noexcept
		: mQueueFamilyIndex(pQueueFamilyIndex)
		, mCommandPool(pCommandPool)
	{ }

	command_pool::command_pool(command_pool&& other) noexcept
		: mQueueFamilyIndex(std::move(other.mQueueFamilyIndex))
		, mCommandPool(std::move(other.mCommandPool))
	{
		other.mQueueFamilyIndex = 0u;
		other.mCommandPool = nullptr;
	}

	command_pool& command_pool::operator=(command_pool&& other) noexcept
	{
		mQueueFamilyIndex = std::move(other.mQueueFamilyIndex);
		mCommandPool = std::move(other.mCommandPool);
		other.mQueueFamilyIndex = 0u;
		other.mCommandPool = nullptr;
		return *this;
	}

	command_pool::~command_pool()
	{
		if (mCommandPool) {
			context().mLogicalDevice.destroyCommandPool(mCommandPool);
			mCommandPool = nullptr;
		}
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
		std::vector<vk::ClearValue> clearValues;
		clearValues.emplace_back(vk::ClearColorValue{ std::array<float,4>{ {0.5f, 0.0f, 0.5f, 1.0f} } });

		auto renderPassBeginInfo = vk::RenderPassBeginInfo()
			.setRenderPass(pRenderPass)
			.setFramebuffer(pFramebuffer)
			.setRenderArea(vk::Rect2D()
						   .setOffset(pOffset)
						   .setExtent(pExtent))
			.setClearValueCount(1u)
			.setPClearValues(clearValues.data());

		mCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		// 2nd parameter: how the drawing commands within the render pass will be provided. It can have one of two values [7]:
		//  - VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
		//  - VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed from secondary command buffers.
	}

	void command_buffer::end_render_pass()
	{
		mCommandBuffer.endRenderPass();
	}

	buffer::buffer() noexcept
		: mSize{ 0u }, mBuffer(nullptr), mMemory(nullptr)
	{ }

	buffer::buffer(size_t pSize, const vk::Buffer& pBuffer, const vk::DeviceMemory& pMemory) noexcept
		: mSize{ pSize }, mBuffer{ pBuffer }, mMemory{ pMemory }
	{ }

	buffer::buffer(buffer&& other) noexcept
		: mSize{ std::move(other.mSize) }, mBuffer{ std::move(other.mBuffer) }, mMemory{ std::move(other.mMemory) }
	{ 
		other.mSize = 0u;
		other.mBuffer = nullptr;
		other.mMemory = nullptr;
	}

	buffer& buffer::operator=(buffer&& other) noexcept
	{
		mSize = std::move(other.mSize);
		mBuffer = std::move(other.mBuffer);
		mMemory = std::move(other.mMemory);
		other.mSize = 0u;
		other.mBuffer = nullptr;
		other.mMemory = nullptr;
		return *this;
	}

	buffer::~buffer()
	{
		if (mBuffer) {
			context().logical_device().destroyBuffer(mBuffer);
			mBuffer = nullptr;
		}
		if (mMemory) {
			context().logical_device().freeMemory(mMemory);
			mMemory = nullptr;
		}
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

		return buffer(pBufferSize, vkBuffer, vkMemory);
	}

	void copy(const buffer& pSource, const buffer& pDestination)
	{
		auto commandBuffer = context().create_command_buffers_for_transfer(1);

		// Immediately start recording the command buffer:
		auto beginInfo = vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		commandBuffer[0].begin_recording();

		auto copyRegion = vk::BufferCopy()
			.setSrcOffset(0u)
			.setDstOffset(0u)
			.setSize(static_cast<vk::DeviceSize>(pSource.mSize));
		commandBuffer[0].mCommandBuffer.copyBuffer(pSource.mBuffer, pDestination.mBuffer, { copyRegion });

		// That's all
		commandBuffer[0].end_recording();

		auto submitInfo = vk::SubmitInfo()
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
		auto buffer = buffer::create(pVertexDataSize * pVertexCount, vk::BufferUsageFlagBits::eVertexBuffer | pAdditionalBufferUsageFlags, pMemoryProperties);

		auto vertexBuffer = vertex_buffer();
		static_cast<cgb::buffer&>(vertexBuffer) = std::move(buffer);
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
		auto buffer = buffer::create(elSize * pIndexCount, vk::BufferUsageFlagBits::eIndexBuffer | pAdditionalBufferUsageFlags, pMemoryProperties);

		auto indexBuffer = index_buffer();
		static_cast<cgb::buffer&>(indexBuffer) = std::move(buffer);
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
		auto buffer = buffer::create(pBufferSize, vk::BufferUsageFlagBits::eUniformBuffer | pAdditionalBufferUsageFlags, pMemoryProperties);

		auto uniformBuffer = uniform_buffer();
		static_cast<cgb::buffer&>(uniformBuffer) = std::move(buffer);
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

	
	// [1] Vulkan Tutorial, Rendering and presentation, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
	// [2] Vulkan Tutorial, Vertex buffer creation, https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
}
