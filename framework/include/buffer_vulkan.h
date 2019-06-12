#pragma once

namespace cgb
{
	/** Represents a Vulkan buffer along with its assigned memory, holds the 
	*	native handle and takes care about lifetime management of the native handles.
	*/
	template <typename Cfg>
	struct buffer
	{
		buffer() noexcept = default;
		buffer(const buffer&) = delete;
		buffer(buffer&& other) noexcept = default;
		buffer& operator=(const buffer&) = delete;
		buffer& operator=(buffer&&) noexcept = default;
		~buffer() {
			mBuffer.release();
			mMemory.release();
		}

		auto size() const						{ return mSize; }
		const auto& config() const				{ return mConfig; }
		const auto& usage_flags() const			{ return mBufferUsageFlags; }
		const auto& buffer_handle() const		{ return mBuffer.get(); }
		const auto& memory_properties() const	{ return mMemoryPropertyFlags; }
		const auto& memory_handle() const		{ return mMemory.get(); }

		size_t mSize;
		Cfg mConfig;
		vk::BufferUsageFlags mBufferUsageFlags;
		vk::UniqueBuffer mBuffer;
		vk::MemoryPropertyFlags mMemoryPropertyFlags;
		vk::UniqueDeviceMemory mMemory;
	};
}
