#pragma once

namespace cgb // ========================== TODO/WIP =================================
{
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
}
