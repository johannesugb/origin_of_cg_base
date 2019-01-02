#pragma once
#include <vulkan/vulkan.hpp>
#include "context_generic_glfw_types.h"

namespace cgb
{
	// FORWARD DECLARATIONS:
	class window;

	/** Represents a native texture handle for the Vulkan context */
	struct texture_handle
	{
		int m_vulkan_specific_handle;
	};

	/** Represents one specific native image format for the Vulkan context */
	struct image_format
	{
		image_format() noexcept;
		image_format(const vk::SurfaceFormatKHR& pSrfFrmt) noexcept;
		image_format(const vk::Format& pFormat, const vk::ColorSpaceKHR& pColorSpace) noexcept;

		vk::Format mFormat;
		vk::ColorSpaceKHR mColorSpace;
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

	/** Struct which stores data for a swap chain */
	struct swap_chain_data
	{
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
		pipeline(const vk::RenderPass&, const vk::PipelineLayout&, const vk::Pipeline&) noexcept;
		pipeline(const pipeline&) = delete;
		pipeline(pipeline&&) noexcept;
		pipeline& operator=(const pipeline&) = delete;
		pipeline& operator=(pipeline&&) noexcept;
		~pipeline();

		vk::RenderPass mRenderPass;
		vk::PipelineLayout mPipelineLayout;
		vk::Pipeline mPipeline;
	};
}
