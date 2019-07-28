#pragma once

namespace cgb
{
	/** Represents a Vulkan framebuffer, holds the native handle and takes
	*	care about their lifetime management.
	*/
	class framebuffer_t
	{
	public:
		framebuffer_t() = default;
		framebuffer_t(const framebuffer_t&) = delete;
		framebuffer_t(framebuffer_t&&) = default;
		framebuffer_t& operator=(const framebuffer_t&) = delete;
		framebuffer_t& operator=(framebuffer_t&&) = default;
		~framebuffer_t() = default;

		static framebuffer_t create(renderpass _Renderpass, std::vector<image_view> _ImageViews, uint32_t _Width, uint32_t _Height, cgb::context_specific_function<void(framebuffer_t&)> pAlterConfigBeforeCreation = {});
		static framebuffer_t create(std::vector<attachment> pAttachments, std::vector<image_view> _ImageViews, uint32_t _Width, uint32_t _Height, cgb::context_specific_function<void(framebuffer_t&)> pAlterConfigBeforeCreation = {});
		static framebuffer_t create(std::vector<image_view> _ImageViews, uint32_t _Width, uint32_t _Height, cgb::context_specific_function<void(framebuffer_t&)> pAlterConfigBeforeCreation = {});

		//// TODO: implement the following:
		//static framebuffer_t create(window* _Window, cgb::context_specific_function<void(framebuffer_t&)> pAlterConfigBeforeCreation = {});

		const auto& render_pass() const { return mRenderpass; }
		const auto& create_info() const { return mCreateInfo; }
		const auto& handle() const { return mFramebuffer.get(); }

	private:
		renderpass mRenderpass;
		std::vector<image_view> mImageViews;
		vk::FramebufferCreateInfo mCreateInfo;
		vk::UniqueFramebuffer mFramebuffer;
		context_tracker<framebuffer_t> mTracker;
	};

	using framebuffer = std::variant<framebuffer_t, std::unique_ptr<framebuffer_t>, std::shared_ptr<framebuffer_t>>;

}
