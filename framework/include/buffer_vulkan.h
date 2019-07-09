#pragma once
#include "buffer_data.h"
#include "synchronization_vulkan.h"

namespace cgb
{
	/** Represents a Vulkan buffer along with its assigned memory, holds the 
	*	native handle and takes care about lifetime management of the native handles.
	*/
	template <typename Cfg>
	class buffer_t
	{
		//template <typename T>
		//friend buffer_t<T> create(T pConfig, vk::BufferUsageFlags pBufferUsage, vk::MemoryPropertyFlags pMemoryProperties);

	public:
		buffer_t() = default;
		buffer_t(const buffer_t&) = delete;
		buffer_t(buffer_t&& other) = default;
		buffer_t& operator=(const buffer_t&) = delete;
		buffer_t& operator=(buffer_t&&) = default;
		~buffer_t() = default; // Declaration order determines destruction order (inverse!)

		const auto& config() const				{ return mConfig; }
		auto size() const						{ return mConfig.total_size(); }
		const auto& memory_properties() const	{ return mMemoryPropertyFlags; }
		const auto& memory_handle() const		{ return mMemory.get(); }
		const auto* memory_handle_addr() const	{ return &mMemory.get(); }
		const auto& buffer_usage_flags() const	{ return mBufferUsageFlags; }
		const auto& buffer_handle() const		{ return mBuffer.get(); }
		const auto* buffer_handle_addr() const	{ return &mBuffer.get(); }

	public: // TODO: Make private
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
		auto vkBuffer = cgb::context().logical_device().createBufferUnique(bufferCreateInfo);

		// The buffer has been created, but it doesn't actually have any memory assigned to it yet. 
		// The first step of allocating memory for the buffer is to query its memory requirements [2]
		auto memRequirements = cgb::context().logical_device().getBufferMemoryRequirements(vkBuffer.get());

		auto allocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(cgb::context().find_memory_type_index(
				memRequirements.memoryTypeBits, 
				pMemoryProperties));

		// Allocate the memory for the buffer:
		auto vkMemory = cgb::context().logical_device().allocateMemoryUnique(allocInfo);

		// If memory allocation was successful, then we can now associate this memory with the buffer
		cgb::context().logical_device().bindBufferMemory(vkBuffer.get(), vkMemory.get(), 0);

		cgb::buffer_t<Cfg> b;
		b.mConfig = pConfig;
		b.mMemoryPropertyFlags = pMemoryProperties;
		b.mMemory = std::move(vkMemory);
		b.mBufferUsageFlags = pBufferUsage;
		b.mBuffer = std::move(vkBuffer);
		return b;
	}

	// Forward-declare what comes after:
	template <typename Cfg>
	cgb::buffer_t<Cfg> create_and_fill(
		Cfg pConfig, 
		cgb::memory_usage pMemoryUsage, 
		const void* pData, 
		std::optional<semaphore>* pSemaphoreOut,
		vk::BufferUsageFlags pUsage);

	// SET DATA
	template <typename Cfg>
	std::optional<semaphore> fill(
		cgb::buffer_t<Cfg>& target,
		const void* pData)
	{
		auto bufferSize = static_cast<vk::DeviceSize>(target.size());

		auto memProps = target.memory_properties();

		// #1: Is our memory on the CPU-SIDE? 
		if (has_flag(memProps, vk::MemoryPropertyFlagBits::eHostVisible)) {
			void* mapped = cgb::context().logical_device().mapMemory(target.memory_handle(), 0, bufferSize);
			memcpy(mapped, pData, bufferSize);
			cgb::context().logical_device().unmapMemory(target.memory_handle());
			// Coherent memory is done; non-coherent memory not yet
			if (!has_flag(memProps, vk::MemoryPropertyFlagBits::eHostCoherent)) {
				// Setup the range 
				auto range = vk::MappedMemoryRange()
					.setMemory(target.memory_handle())
					.setOffset(0)
					.setSize(bufferSize);
				// Flush the range
				cgb::context().logical_device().flushMappedMemoryRanges(1, &range);
			}
			// TODO: Handle has_flag(memProps, vk::MemoryPropertyFlagBits::eHostCached) case

			return std::nullopt; // TODO: This should be okay, is it?
		}

		// #2: Otherwise, it must be on the GPU-SIDE!
		else {
			assert(has_flag(memProps, vk::MemoryPropertyFlagBits::eDeviceLocal));

			// Okay, we have to create a (somewhat temporary) staging buffer and transfer it to the GPU
			// "somewhat temporary" means that it can not be deleted in this function, but only
			//						after the transfer operation has completed => handle via semaphore!
			auto stagingBuffer = create_and_fill(
				generic_buffer_data{ target.size() }, 
				cgb::memory_usage::host_coherent, 
				pData, 
				nullptr, //< TODO: What about the semaphore???
				vk::BufferUsageFlagBits::eTransferSrc);

			auto commandBuffer = cgb::context().transfer_queue().pool().get_command_buffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
			commandBuffer.begin_recording();

			auto copyRegion = vk::BufferCopy{}
				.setSrcOffset(0u)
				.setDstOffset(0u)
				.setSize(bufferSize);
			commandBuffer.handle().copyBuffer(stagingBuffer.buffer_handle(), target.buffer_handle(), { copyRegion });

			// That's all
			commandBuffer.end_recording();

			auto submitInfo = vk::SubmitInfo{}
				.setCommandBufferCount(1u)
				.setPCommandBuffers(commandBuffer.handle_addr());
			cgb::context().transfer_queue().handle().submit({ submitInfo }, nullptr); // not using fence... TODO: maybe use fence!
			cgb::context().transfer_queue().handle().waitIdle();

			// TODO: Handle has_flag(memProps, vk::MemoryPropertyFlagBits::eHostCached) case

			return std::nullopt; // TODO: Create a semaphore FFS!
		}
	}

	// CREATE 
	template <typename Cfg>
	cgb::buffer_t<Cfg> create(
		Cfg pConfig,
		cgb::memory_usage pMemoryUsage,
		vk::BufferUsageFlags pUsage = vk::BufferUsageFlags())
	{
		auto bufferSize = pConfig.total_size();


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
			pUsage |= vk::BufferUsageFlagBits::eTransferDst; // We have to transfer data into it, because we are inside of `cgb::create_and_fill`
			break;
		case cgb::memory_usage::device_readback:
			memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
			pUsage |= vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
			break;
		case cgb::memory_usage::device_protected:
			memoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eProtected;
			pUsage |= vk::BufferUsageFlagBits::eTransferDst;
			break;
		}

		if constexpr (std::is_same_v<Cfg, cgb::uniform_buffer_data>) {
			pUsage |= vk::BufferUsageFlagBits::eUniformBuffer;
		}
		if constexpr (std::is_same_v<Cfg, cgb::uniform_texel_buffer_data>) {
			pUsage |= vk::BufferUsageFlagBits::eUniformTexelBuffer;
		}
		if constexpr (std::is_same_v<Cfg, cgb::storage_buffer_data>) {
			pUsage |= vk::BufferUsageFlagBits::eStorageBuffer;
		}
		if constexpr (std::is_same_v<Cfg, cgb::storage_texel_buffer_data>) {
			pUsage |= vk::BufferUsageFlagBits::eStorageTexelBuffer;
		}
		if constexpr (std::is_same_v<Cfg, cgb::vertex_buffer_data>) {
			pUsage |= vk::BufferUsageFlagBits::eVertexBuffer;
		}
		if constexpr (std::is_same_v<Cfg, cgb::index_buffer_data>) {
			pUsage |= vk::BufferUsageFlagBits::eIndexBuffer;
		}

		// Create buffer here to make use of named return value optimization.
		// How it will be filled depends on where the memory is located at.
		auto result = cgb::create(pConfig, pUsage, memoryFlags);
		return result;
	}

	/** Create multiple buffers */
	template <typename Cfg>
	auto create_multiple(
		int pNumBuffers,
		Cfg pConfig,
		cgb::memory_usage pMemoryUsage,
		vk::BufferUsageFlags pUsage = vk::BufferUsageFlags())
	{
		std::vector<buffer_t<Cfg>> bs;
		bs.reserve(pNumBuffers);
		for (int i = 0; i < pNumBuffers; ++i) {
			bs.push_back(create(pConfig, pMemoryUsage, pUsage));
		}
		return bs;
	}

	// CREATE AND FILL
	template <typename Cfg>
	cgb::buffer_t<Cfg> create_and_fill(
		Cfg pConfig, 
		cgb::memory_usage pMemoryUsage, 
		const void* pData, 
		std::optional<semaphore>* pSemaphoreOut = nullptr,
		vk::BufferUsageFlags pUsage = vk::BufferUsageFlags())
	{
		// Does NRVO also apply here? I hope so...
		auto result = create(pConfig, pMemoryUsage, pUsage);

		auto semaphore = fill(result, pData);
		if (semaphore.has_value()) {
			// If we have a semaphore, we have to do something with it!
			if (nullptr != pSemaphoreOut) {
				*pSemaphoreOut = std::move(*semaphore);
			}
			else {
				LOG_ERROR("An unhandled semaphore emerged in create_and_fill. You have to take care of it!");
			}
		}
		
		return result;
	}
}