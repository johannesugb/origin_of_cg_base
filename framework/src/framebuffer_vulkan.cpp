namespace cgb
{
	const vk::RenderPass& framebuffer_t::render_pass_handle() const
	{
		if (std::holds_alternative<renderpass>(mRenderpass)) {
			return cgb::get(std::get<renderpass>(mRenderpass)).handle();
		}
		assert(std::holds_alternative<vk::RenderPass>(mRenderpass));
		return std::get<vk::RenderPass>(mRenderpass);
	}

	void framebuffer_t::finish_configuration(std::vector<const image_view_t*> _ImageViews, uint32_t _Width, uint32_t _Height, cgb::context_specific_function<void(framebuffer_t&)> _AlterConfigBeforeCreation)
	{
		std::vector<vk::ImageView> imageViewHandles;
		for (const auto* iv : _ImageViews) {
			imageViewHandles.push_back(iv->view_handle());
		}

		mCreateInfo = vk::FramebufferCreateInfo{}
			.setRenderPass(render_pass_handle())
			.setAttachmentCount(static_cast<uint32_t>(imageViewHandles.size()))
			.setPAttachments(imageViewHandles.data())
			.setWidth(_Width)
			.setHeight(_Height)
			// TODO: Support multiple layers of image arrays!
			.setLayers(1u); // number of layers in image arrays [6]

		// Maybe alter the config?!
		if (_AlterConfigBeforeCreation.mFunction) {
			_AlterConfigBeforeCreation.mFunction(*this);
		}

		mTracker.setTrackee(*this);
		mFramebuffer = context().logical_device().createFramebufferUnique(mCreateInfo);
	}

	framebuffer_t framebuffer_t::create(const renderpass_t& _Renderpass, std::vector<const image_view_t*> _ImageViews, uint32_t _Width, uint32_t _Height, cgb::context_specific_function<void(framebuffer_t&)> _AlterConfigBeforeCreation)
	{
		framebuffer_t result;
		result.mRenderpass = _Renderpass.handle();
		result.finish_configuration(std::move(_ImageViews), _Width, _Height, std::move(_AlterConfigBeforeCreation));
		return result;
	}

	framebuffer_t framebuffer_t::create(renderpass _Renderpass, std::vector<image_view> _ImageViews, uint32_t _Width, uint32_t _Height, cgb::context_specific_function<void(framebuffer_t&)> _AlterConfigBeforeCreation)
	{
		framebuffer_t result;
		result.mRenderpass = std::move(_Renderpass);
		result.mImageViews = std::move(_ImageViews);

		std::vector<const image_view_t*> ivPtrs;
		for (auto& imView : result.mImageViews) {
			ivPtrs.push_back(&cgb::get(imView));
		}

		result.finish_configuration(ivPtrs, _Width, _Height, std::move(_AlterConfigBeforeCreation));
		return result;
	}

	framebuffer_t framebuffer_t::create(std::vector<attachment> pAttachments, std::vector<image_view> _ImageViews, uint32_t _Width, uint32_t _Height, cgb::context_specific_function<void(framebuffer_t&)> _AlterConfigBeforeCreation)
	{
		return framebuffer_t::create(
			renderpass_t::create(std::move(pAttachments)),
			std::move(_ImageViews),
			_Width, _Height,
			std::move(_AlterConfigBeforeCreation)
		);
	}

	framebuffer_t framebuffer_t::create(std::vector<image_view> _ImageViews, uint32_t _Width, uint32_t _Height, cgb::context_specific_function<void(framebuffer_t&)> _AlterConfigBeforeCreation)
	{
		// Create attachments based on the given image views
		std::vector<attachment> attachments;
		uint32_t  location = 0u;
		for (const auto& iv : _ImageViews) {
			attachments.push_back(attachment::create_for(cgb::get(iv), location++));
		}

		// Hope that helps:
		return framebuffer_t::create(std::move(attachments), std::move(_ImageViews), _Width, _Height, std::move(_AlterConfigBeforeCreation));
	}

}
