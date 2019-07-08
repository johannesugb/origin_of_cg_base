#include "context_vulkan_types.h"

namespace cgb
{

#pragma region command_buffer
	std::vector<command_buffer> command_buffer::create_many(uint32_t pCount, command_pool& pPool, vk::CommandBufferUsageFlags pUsageFlags)
	{
		auto bufferAllocInfo = vk::CommandBufferAllocateInfo()
			.setCommandPool(pPool.handle())
			.setLevel(vk::CommandBufferLevel::ePrimary) // Those, allocated from a pool, are primary command buffers; secondary command buffers can be allocated from command buffers.
			.setCommandBufferCount(pCount);

		auto tmp = context().logical_device().allocateCommandBuffersUnique(bufferAllocInfo);
		
		// Iterate over all the "raw"-Vk objects in `tmp` and...
		std::vector<command_buffer> buffers;
		buffers.reserve(pCount);
		std::transform(std::begin(tmp), std::end(tmp),
			std::back_inserter(buffers),
			// ...transform them into `cgb::command_buffer` objects:
			[usageFlags = pUsageFlags](auto& vkCb) {
				command_buffer result;
				result.mBeginInfo = vk::CommandBufferBeginInfo()
					.setFlags(usageFlags)
					.setPInheritanceInfo(nullptr);
				result.mCommandBuffer = std::move(vkCb);
				return result;
			});
		return buffers;
	}

	command_buffer command_buffer::create(command_pool& pPool, vk::CommandBufferUsageFlags pUsageFlags)
	{
		auto result = std::move(command_buffer::create_many(1, pPool, pUsageFlags)[0]);
		return result;
	}
#pragma endregion

#pragma region command_pool
	command_pool command_pool::create(uint32_t pQueueFamilyIndex, vk::CommandPoolCreateFlags pCreateFlags)
	{
		auto createInfo = vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(pQueueFamilyIndex)
			.setFlags(pCreateFlags); // Optional
		// Possible values for the flags [7]
		//  - VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
		//  - VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
		command_pool result;
		result.mQueueFamilyIndex = pQueueFamilyIndex;
		result.mCreateInfo = createInfo;
		result.mCommandPool = context().logical_device().createCommandPoolUnique(createInfo);
		return result;
	}

	std::vector<command_buffer> command_pool::get_command_buffers(uint32_t pCount, vk::CommandBufferUsageFlags pUsageFlags)
	{
		return command_buffer::create_many(pCount, *this, pUsageFlags);
	}

	command_buffer command_pool::get_command_buffer(vk::CommandBufferUsageFlags pUsageFlags)
	{
		return command_buffer::create(*this, pUsageFlags);
	}
#pragma endregion

#pragma region device_queue
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
		auto& prepd_queue = sPreparedQueues.emplace_back();
		prepd_queue.mQueueFamilyIndex = familyIndex;
		prepd_queue.mQueueIndex = queueIndex;
		prepd_queue.mPriority = 0.5f; // default priority of 0.5
		prepd_queue.mQueue = nullptr;
		return &prepd_queue;
	}

	device_queue device_queue::create(uint32_t pQueueFamilyIndex, uint32_t pQueueIndex)
	{
		device_queue result;
		result.mQueueFamilyIndex = pQueueFamilyIndex;
		result.mQueueIndex = pQueueIndex;
		result.mPriority = 0.5f; // default priority of 0.5f
		result.mQueue = context().logical_device().getQueue(result.mQueueFamilyIndex, result.mQueueIndex);
		return result;
	}

	device_queue device_queue::create(const device_queue& pPreparedQueue)
	{
		device_queue result;
		result.mQueueFamilyIndex = pPreparedQueue.family_index();
		result.mQueueIndex = pPreparedQueue.queue_index();
		result.mPriority = pPreparedQueue.mPriority; // default priority of 0.5f
		result.mQueue = context().logical_device().getQueue(result.mQueueFamilyIndex, result.mQueueIndex);
		return result;
	}

	command_pool& device_queue::pool() const 
	{ 
		return context().get_command_pool_for_queue(*this); 
	}
#pragma endregion














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


	


	void command_buffer::begin_recording()
	{
		mCommandBuffer->begin(mBeginInfo);
	}

	void command_buffer::end_recording()
	{
		mCommandBuffer->end();
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
		
		mCommandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		// 2nd parameter: how the drawing commands within the render pass will be provided. It can have one of two values [7]:
		//  - VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
		//  - VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed from secondary command buffers.
	}

	void command_buffer::set_image_barrier(const vk::ImageMemoryBarrier& pBarrierInfo)
	{
		mCommandBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::DependencyFlags(),
			{}, {}, { pBarrierInfo });
	}

	void command_buffer::copy_image(const image_t& pSource, const vk::Image& pDestination)
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
		mCommandBuffer->copyImage(pSource.mImage, vk::ImageLayout::eTransferSrcOptimal, pDestination, vk::ImageLayout::eTransferDstOptimal, { copyInfo });
	}

	void command_buffer::end_render_pass()
	{
		mCommandBuffer->endRenderPass();
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

	vk::ImageMemoryBarrier image_t::create_barrier(vk::AccessFlags pSrcAccessMask, vk::AccessFlags pDstAccessMask, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, std::optional<vk::ImageSubresourceRange> pSubresourceRange) const
	{
		return create_image_barrier(mImage, mInfo.format, pSrcAccessMask, pDstAccessMask, pOldLayout, pNewLayout, pSubresourceRange);
	}

	void transition_image_layout(const image_t& pImage, vk::Format pFormat, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout)
	{
		//auto commandBuffer = context().create_command_buffers_for_graphics(1, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		auto commandBuffer = context().graphics_queue().pool().get_command_buffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		// Immediately start recording the command buffer:
		commandBuffer.begin_recording();

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
		commandBuffer.handle().pipelineBarrier(
			sourceStageFlags,
			destinationStageFlags,
			vk::DependencyFlags(), // The third parameter is either 0 or VK_DEPENDENCY_BY_REGION_BIT. The latter turns the barrier into a per-region condition. That means that the implementation is allowed to already begin reading from the parts of a resource that were written so far, for example. [3]
			{},
			{},
			{ barrier });

		// That's all
		commandBuffer.end_recording();
		
		auto submitInfo = vk::SubmitInfo()
			.setCommandBufferCount(1u)
			.setPCommandBuffers(commandBuffer.handle_addr());
		cgb::context().graphics_queue().handle().submit({ submitInfo }, nullptr); // not using fence... TODO: maybe use fence!
		cgb::context().graphics_queue().handle().waitIdle();
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

		//return acceleration_structure(accInfo, accStructure, handle, memPropertyFlags, deviceMemory);
		return acceleration_structure{};
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
	{ }

	shader_binding_table::shader_binding_table(size_t pSize, const vk::BufferUsageFlags& pBufferFlags, const vk::Buffer& pBuffer, const vk::MemoryPropertyFlags& pMemoryProperties, const vk::DeviceMemory& pMemory) noexcept
	{ }

	shader_binding_table::shader_binding_table(shader_binding_table&& other) noexcept
	{ }

	shader_binding_table& shader_binding_table::operator=(shader_binding_table&& other) noexcept
	{ 
		return *this;
	}

	shader_binding_table::~shader_binding_table()
	{ }

	shader_binding_table shader_binding_table::create(const pipeline& pRtPipeline)
	{
		auto numGroups = 5u; // TODO: store groups in `pipeline` (or rather in `ray_tracing_pipeline : pipeline`) and then, read from pRtPipeline
		auto rtProps = context().get_ray_tracing_properties();
		auto shaderBindingTableSize = rtProps.shaderGroupHandleSize * numGroups;

		// TODO: Use *new* buffer_t
		//auto b = buffer::create(shaderBindingTableSize,
		//						vk::BufferUsageFlagBits::eTransferSrc,
		//						vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		//void* mapped = context().logical_device().mapMemory(b.mMemory, 0, b.mSize);
		//// Transfer something into the buffer's memory...
		//context().logical_device().getRayTracingShaderGroupHandlesNV(pRtPipeline.mPipeline, 0, numGroups, b.mSize, mapped, context().dynamic_dispatch());
		//context().logical_device().unmapMemory(b.mMemory);
		
		auto sbt = shader_binding_table();
		//static_cast<buffer&>(sbt) = std::move(b);
		return sbt;
	}



	// [1] Vulkan Tutorial, Rendering and presentation, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
	// [2] Vulkan Tutorial, Vertex buffer creation, https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
	// [3] Vulkan Tutorial, Images, https://vulkan-tutorial.com/Texture_mapping/Images
	// [4] Vulkan Tutorial, Image view and sampler, https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler
}
