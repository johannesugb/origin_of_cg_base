namespace cgb
{
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
}
