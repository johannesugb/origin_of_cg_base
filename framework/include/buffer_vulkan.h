#pragma once

namespace cgb
{
	/** Represents a Vulkan buffer along with its assigned memory, holds the 
	*	native handle and takes care about lifetime management of the native handles.
	*/
	template <typename Cfg>
	class buffer_t
	{
	public:
		buffer_t() = default;
		buffer_t(const buffer_t&) = delete;
		buffer_t(buffer_t&& other) = default;
		buffer_t& operator=(const buffer_t&) = delete;
		buffer_t& operator=(buffer_t&&) = default;
		~buffer_t() = default; // Declaration order determines destruction order (inverse!)

		auto size() const						{ return mSize; }
		const auto& config() const				{ return mConfig; }
		const auto& memory_properties() const	{ return mMemoryPropertyFlags; }
		const auto& memory_handle() const		{ return mMemory.get(); }
		const auto* memory_handle_addr() const	{ return &mMemory.get(); }
		const auto& buffer_usage_flags() const	{ return mBufferUsageFlags; }
		const auto& buffer_handle() const		{ return mBuffer.get(); }
		const auto* buffer_handle_addr() const	{ return &mBuffer.get(); }

	private:
		size_t mSize;
		Cfg mConfig;
		vk::MemoryPropertyFlags mMemoryPropertyFlags;
		vk::UniqueDeviceMemory mMemory;
		vk::BufferUsageFlags mBufferUsageFlags;
		vk::UniqueBuffer mBuffer;
	};


	/**	Create a buffer which is always created with exclusive access for a queue.
	*	If different queues are being used, ownership has to be transferred explicitely.
	*/
	template <typename Cfg>
	buffer_t<Cfg> create(Cfg pConfig, vk::BufferUsageFlags pBufferUsage, vk::MemoryPropertyFlags pMemoryProperties)
	{
		auto bufferSize = pConfig.total_size();

		// Create (possibly multiple) buffer(s):
		auto bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(static_cast<vk::DeviceSize>(bufferSize))
			.setUsage(pBufferUsage)
			// Always grant exclusive ownership to the queue.
			.setSharingMode(vk::SharingMode::eExclusive)
			// The flags parameter is used to configure sparse buffer memory, which is not relevant right now. We'll leave it at the default value of 0. [2]
			.setFlags(vk::BufferCreateFlags()); 

		// Create the buffer on the logical device
		auto vkBuffer = context().logical_device().createBufferUnique(bufferCreateInfo);

		// The buffer has been created, but it doesn't actually have any memory assigned to it yet. 
		// The first step of allocating memory for the buffer is to query its memory requirements [2]
		auto memRequirements = context().logical_device().getBufferMemoryRequirements(vkBuffer);

		auto allocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(context().find_memory_type_index(
				memRequirements.memoryTypeBits, 
				pMemoryProperties));

		// Allocate the memory for the buffer:
		auto vkMemory = context().logical_device().allocateMemoryUnique(allocInfo);

		cgb::buffer_t<Cfg> b;
		b.mSize = bufferSize;
		b.mConfig = pConfig;
		b.mMemoryPropertyFlags = pMemoryProperties;
		b.mMemory = std::move(vkMemory);
		b.mBufferUsageFlags = pBufferUsage;
		b.mBuffer = std::move(vkBuffer);
		return b;
	}

	/** Create multiple buffers */
	template <typename Cfg>
	std::vector<buffer_t<Cfg>> create(Cfg pConfig, vk::BufferUsageFlags pBufferUsage, vk::MemoryPropertyFlags pMemoryProperties, uint32_t pNumBuffers)
	{
		std::vector<buffer_t<Cfg>> bs;
		bs.reserve(pNumBuffers);
		for (uint32_t = 0; i < pNumBuffers; ++i) {
			bs.push_back(create(pConfig, pBufferUsage, pMemoryProperties));
		}
		return bs;
	}

	template <typename Cfg>
	std::tuple<cgb::buffer_t<Cfg>, semaphore> create_and_fill(
		Cfg pConfig, 
		cgb::memory_usage pMemoryUsage, 
		const void* pData, 
		cgb::buffer_usage pUsage = cgb::buffer_usage::nothing_special)
	{
		auto bufferSize = pConfig.total_size();

		vk::BufferUsageFlags usageFlags;
		if ((pUsage & cgb::buffer_usage::indirect) == cgb::buffer_usage::indirect)										{ usageFlags |= vk::BufferUsageFlagBits::eIndirectBuffer; }
		if ((pUsage & cgb::buffer_usage::transform_feedback) == cgb::buffer_usage::transform_feedback)					{ usageFlags |= vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT; }
		if ((pUsage & cgb::buffer_usage::transform_feedback_counter) == cgb::buffer_usage::transform_feedback_counter)	{ usageFlags |= vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT; }
		if ((pUsage & cgb::buffer_usage::conditional_rendering) == cgb::buffer_usage::conditional_rendering)			{ usageFlags |= vk::BufferUsageFlagBits::eConditionalRenderingEXT; }
		if ((pUsage & cgb::buffer_usage::ray_tracing) == cgb::buffer_usage::ray_tracing)								{ usageFlags |= vk::BufferUsageFlagBits::eRayTracingNV; }
		if ((pUsage & cgb::buffer_usage::device_address) == cgb::buffer_usage::device_address)							{ usageFlags |= vk::BufferUsageFlags(); /* TODO: use the proper flag! */ }

		vk::MemoryPropertyFlags memoryFlags;
		// We've got two major branches here: 
		// 1) Memory will stay on the host and there will be no dedicated memory on the device
		// 2) Memory will be transfered to the device. (Only in this case, we'll need to create semaphores.)
		switch (pMemoryUsage)
		{
		case cgb::memory_usage::host_visible:
			memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible;
			break;
		case cgb::memory_usage::host_coherent:
			memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			break;
		case cgb::memory_usage::host_cached:
			memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached;
			break;
		case cgb::memory_usage::device:
			memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
			break;
		case cgb::memory_usage::device_protected:
			memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eProtected;
			break;
		}

		// Create buffer here to make use of named return value optimization.
		// How it will be filled depends on where the memory is located at.
		auto result = cgb::create(pConfig, usageFlags, memoryFlags);

		switch (pMemoryUsage)
		{
		case cgb::memory_usage::host_visible:
		case cgb::memory_usage::host_coherent:
		case cgb::memory_usage::host_cached:
			{
				// TODO: Proceed here!	

				auto memory = buffer.memory_handle_at(i);
				void* mapped = context().logical_device().mapMemory(memory, 0, bufferSize);
				memcpy(mapped, pData, copySize);
				context().logical_device().unmapMemory(memory);
			}
			break;
		case cgb::memory_usage::device:
		case cgb::memory_usage::device_protected:
			{
				// TODO: Create a staging buffer, fill it (maybe with flush?)
				// Create another device buffer
				// Create the staging buffer contents into device buffer

			}
			break;
		}


		return result;
	}
}
