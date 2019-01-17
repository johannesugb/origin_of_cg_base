// hello_world.cpp : Defines the entry point for the console application.
//
#include "cg_base.h"
using namespace std;

class hello_behavior : public cgb::cg_element
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static vk::VertexInputBindingDescription binding_description()
		{
			return vk::VertexInputBindingDescription()
				.setBinding(0u)
				.setStride(sizeof(Vertex))
				.setInputRate(vk::VertexInputRate::eVertex);
		}

		static auto attribute_descriptions()
		{
			static std::array attribDescs = {
				vk::VertexInputAttributeDescription()
					.setBinding(0u)
					.setLocation(0u)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(static_cast<uint32_t>(offsetof(Vertex, pos)))
				,
				vk::VertexInputAttributeDescription()
					.setBinding(0u)
					.setLocation(1u)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(static_cast<uint32_t>(offsetof(Vertex, color)))
				,
				vk::VertexInputAttributeDescription()
					.setBinding(0u)
					.setLocation(2u)
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(static_cast<uint32_t>(offsetof(Vertex, texCoord)))
			};
			return attribDescs;
		}
	};

	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct GeometryInstance
	{
		glm::mat4 transform;
		uint32_t instanceId : 24;
		uint32_t mask : 8;
		uint32_t instanceOffset : 24;
		uint32_t flags : 8;
		uint64_t accelerationStructureHandle;
	};

public:
	hello_behavior(cgb::window* pMainWnd) 
		: mMainWnd(pMainWnd)
		, mVertices({	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
						{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
						{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
						{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

						{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
						{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
						{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
						{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}} })
		, mIndices({ {	0, 1, 2, 2, 3, 0,
						4, 5, 6, 6, 7, 4 } })
	{ 
	}

#ifdef USE_VULKAN_CONTEXT
	void create_vertex_buffer()
	{
		auto stagingBuffer = cgb::buffer::create(
			sizeof(mVertices[0]) * mVertices.size(),
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		// Filling the staging buffer!
		//   This is done by mapping the buffer memory into CPU accessible memory with vkMapMemory. [2]
		stagingBuffer.fill_host_coherent_memory(mVertices.data());

		mVertexBuffer = cgb::vertex_buffer::create(
			sizeof(mVertices[0]), mVertices.size(),
			vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

		// Transfer the data from the staging buffer into the vertex buffer
		cgb::copy(stagingBuffer, mVertexBuffer);
	}

	void create_index_buffer()
	{
		auto stagingBuffer = cgb::buffer::create(
			sizeof(mIndices[0]) * mIndices.size(),
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		
		stagingBuffer.fill_host_coherent_memory(mIndices.data());

		mIndexBuffer = cgb::index_buffer::create(
			vk::IndexType::eUint16, mIndices.size(),
			vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

		cgb::copy(stagingBuffer, mIndexBuffer);
	}

	void load_model()
	{
		mModel = cgb::Model::LoadFromFile("assets/chalet.obj", glm::mat4(1.0f));
		auto& mesh = mModel->mesh_at(0);
		
		{
			auto stagingBuffer = cgb::buffer::create(
				mesh.m_vertex_data.size(),
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			// Filling the staging buffer!
			//   This is done by mapping the buffer memory into CPU accessible memory with vkMapMemory. [2]
			stagingBuffer.fill_host_coherent_memory(mesh.m_vertex_data.data());

			mModelVertices = cgb::vertex_buffer::create(
				mesh.m_size_one_vertex, mesh.m_vertex_data.size() / mesh.m_size_one_vertex,
				vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			// Transfer the data from the staging buffer into the vertex buffer
			cgb::copy(stagingBuffer, mModelVertices);
		}

		{
			auto stagingBuffer = cgb::buffer::create(
				sizeof(mesh.m_indices[0]) * mesh.m_indices.size(),
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			stagingBuffer.fill_host_coherent_memory(mesh.m_indices.data());

			mModelIndices = cgb::index_buffer::create(
				vk::IndexType::eUint32, mesh.m_indices.size(),
				vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			cgb::copy(stagingBuffer, mModelIndices);
		}
	}

	void create_uniform_buffers()
	{
		for (auto i = 0; i < mFrameBuffers.size(); ++i) {
			mUniformBuffers.push_back(cgb::uniform_buffer::create(
				sizeof(UniformBufferObject),
				vk::BufferUsageFlags(),
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		}
	}

	void create_descriptor_set_layout()
	{
		std::array bindings = { 
			vk::DescriptorSetLayoutBinding()
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1u)
				.setStageFlags(vk::ShaderStageFlagBits::eVertex)
				.setPImmutableSamplers(nullptr) // The pImmutableSamplers field is only relevant for image sampling related descriptors [3]
			,
			vk::DescriptorSetLayoutBinding()
				.setBinding(1u)
				.setDescriptorCount(1u)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setPImmutableSamplers(nullptr)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment)
		};

		auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(static_cast<uint32_t>(bindings.size()))
			.setPBindings(bindings.data());

		mDescriptorSetLayout = cgb::descriptor_set_layout::create(descriptorSetLayoutCreateInfo);
	}

	void create_rt_descriptor_set_layout()
	{
		// CREATE DESCRIPTOR SET LAYOUT:
		std::array descriptorSetLayoutBindings = {
			// Acceleration Structure Layout Binding:
			vk::DescriptorSetLayoutBinding()
			.setBinding(0u)
			.setDescriptorType(vk::DescriptorType::eAccelerationStructureNV)
			.setDescriptorCount(1u)
			.setStageFlags(vk::ShaderStageFlagBits::eRaygenNV)
			,
			// Output Image Layout Binding:
			vk::DescriptorSetLayoutBinding()
			.setBinding(1u)
			.setDescriptorType(vk::DescriptorType::eStorageImage)
			.setDescriptorCount(1u)
			.setStageFlags(vk::ShaderStageFlagBits::eRaygenNV)
		};

		auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setFlags(vk::DescriptorSetLayoutCreateFlags())
			.setBindingCount(static_cast<uint32_t>(descriptorSetLayoutBindings.size()))
			.setPBindings(descriptorSetLayoutBindings.data());

		mRtDescriptorSetLayout = cgb::descriptor_set_layout::create(descriptorSetLayoutCreateInfo);
	}

	void create_descriptor_sets()
	{
		std::vector<vk::DescriptorSetLayout> layouts;
		for (int i = 0; i < mFrameBuffers.size(); ++i) {
			layouts.push_back(mDescriptorSetLayout.mDescriptorSetLayout);
		}
		mDescriptorSets = cgb::context().create_descriptor_set(layouts);

		for (auto i = 0; i < mDescriptorSets.size(); ++i) {
			auto bufferInfo = vk::DescriptorBufferInfo()
				.setBuffer(mUniformBuffers[i].mBuffer)
				.setOffset(0)
				.setRange(sizeof(UniformBufferObject));
			auto descriptorWriteBuffer = vk::WriteDescriptorSet()
				.setDstSet(mDescriptorSets[i].mDescriptorSet)
				.setDstBinding(0u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1u)
				.setPBufferInfo(&bufferInfo);

			auto imageInfo = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(mImageView.mImageView)
				.setSampler(mSampler.mSampler);
			auto descriptorWriteSampler = vk::WriteDescriptorSet()
				.setDstSet(mDescriptorSets[i].mDescriptorSet)
				.setDstBinding(1u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1u)
				.setPImageInfo(&imageInfo);

			cgb::context().logical_device().updateDescriptorSets({ descriptorWriteBuffer, descriptorWriteSampler }, {});
		}
	}

	void create_texture_image()
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* pixels = stbi_load("assets/chalet.jpg", &width, &height, &channels, STBI_rgb_alpha);
		size_t imageSize = width * height * 4;

		if (!pixels) {
			throw std::runtime_error("Couldnt load image using stbi_load");
		}

		auto stagingBuffer = cgb::buffer::create(
			imageSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		// Copy texture into staging buffer
		stagingBuffer.fill_host_coherent_memory(pixels);

		stbi_image_free(pixels);

		auto img = cgb::image::create2D(width, height);
		cgb::transition_image_layout(img, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		cgb::copy_buffer_to_image(stagingBuffer, img);
		cgb::transition_image_layout(img, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
		mImage = std::make_shared<cgb::image>(std::move(img));
	}

	void create_depth_buffer()
	{
		// Select a suitable format
		vk::Format selectedFormat;
		std::array desiredFormats = { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };
		for (auto& f : desiredFormats) {
			if (cgb::context().is_format_supported(f, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment)) {
				selectedFormat = f;
				break;
			}
		}

		mDepthImage = std::make_shared<cgb::image>(std::move(cgb::image::create2D(
			mSwapChainData->mSwapChainExtent.width, mSwapChainData->mSwapChainExtent.height,
			selectedFormat,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal)));
		mDepthImageView = cgb::image_view::create(mDepthImage, selectedFormat, vk::ImageAspectFlagBits::eDepth);
		cgb::transition_image_layout(*mDepthImage, selectedFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	}

	void create_rt_geometry()
	{
		mGeometries.emplace_back()
			.setGeometryType(vk::GeometryTypeNV::eTriangles)
			.setGeometry(vk::GeometryDataNV()
						 .setTriangles(vk::GeometryTrianglesNV()
									   .setVertexData(mVertexBuffer.mBuffer)
									   .setVertexOffset(0)
									   .setVertexCount(mVertexBuffer.mVertexCount)
									   .setVertexStride(mVertexBuffer.mSize / mVertexBuffer.mVertexCount)
									   .setVertexFormat(vk::Format::eR32G32B32Sfloat)
									   .setIndexData(mIndexBuffer.mBuffer)
									   .setIndexOffset(0)
									   .setIndexCount(mIndexBuffer.mIndexCount)
									   .setIndexType(mIndexBuffer.mIndexType)
									   .setTransformData(nullptr)
									   .setTransformOffset(0)));
	}

	void create_rt_geometry_instances()
	{
		auto& inst = mGeometryInstances.emplace_back();
		inst.transform = glm::mat4(1.0f);
		inst.instanceId = 0;
		inst.mask = 0xff;
		inst.instanceOffset = 0;
		inst.flags = static_cast<uint32_t>(vk::GeometryInstanceFlagBitsNV::eTriangleCullDisable);
		inst.accelerationStructureHandle = mBottomLevelAccStructure.mHandle.mHandle;

		auto& bfr = mGeometryInstanceBuffers.emplace_back(cgb::buffer::create(
			sizeof(GeometryInstance),
			vk::BufferUsageFlagBits::eRayTracingNV,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		bfr.fill_host_coherent_memory(&inst);
	}

	void build_acceleration_structures()
	{
		auto scratchBuffer = cgb::buffer::create(std::max(mBottomLevelAccStructure.get_scratch_buffer_size(), mTopLevelAccStructure.get_scratch_buffer_size()),
												 vk::BufferUsageFlagBits::eRayTracingNV,
												 vk::MemoryPropertyFlagBits::eDeviceLocal);

		auto commandBuffers = cgb::context().create_command_buffers_for_graphics(1, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		commandBuffers[0].begin_recording();

		auto memoryBarrier = vk::MemoryBarrier()
			.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteNV | vk::AccessFlagBits::eAccelerationStructureReadNV)
			.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteNV | vk::AccessFlagBits::eAccelerationStructureReadNV);

		// Build BLAS
		commandBuffers[0].mCommandBuffer.buildAccelerationStructureNV(
			mBottomLevelAccStructure.mAccStructureInfo,
			nullptr, 0,								// no instance data for bottom level AS
			VK_FALSE,								// update = false
			mBottomLevelAccStructure.mAccStructure, // destination AS
			nullptr,								// no source AS
			scratchBuffer.mBuffer, 0,				// scratch buffer + offset
			cgb::context().dynamic_dispatch());

		// Barrier
		commandBuffers[0].mCommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eAccelerationStructureBuildNV,
			vk::PipelineStageFlagBits::eAccelerationStructureBuildNV,
			vk::DependencyFlags(),
			{ memoryBarrier }, 
			{}, {});

		// Build TLAS
		commandBuffers[0].mCommandBuffer.buildAccelerationStructureNV(
			mTopLevelAccStructure.mAccStructureInfo,
			mGeometryInstanceBuffers[0].mBuffer, 0,	// buffer containing the instance data (only one)
			VK_FALSE,								// update = false
			mTopLevelAccStructure.mAccStructure,	// destination AS
			nullptr,								// no source AS
			scratchBuffer.mBuffer, 0,				// scratch buffer + offset
			cgb::context().dynamic_dispatch());

		// Barrier
		commandBuffers[0].mCommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eAccelerationStructureBuildNV,
			vk::PipelineStageFlagBits::eRayTracingShaderNV,
			vk::DependencyFlags(),
			{ memoryBarrier },
			{}, {});

		commandBuffers[0].end_recording();
		auto submitInfo = vk::SubmitInfo()
			.setCommandBufferCount(1u)
			.setPCommandBuffers(&commandBuffers[0].mCommandBuffer);
		cgb::context().graphics_queue().submit({ submitInfo }, nullptr); 
		cgb::context().graphics_queue().waitIdle();
	}

	void create_rt_descriptor_set()
	{
		// create an offscreen image for each one:
		for (int i = 0; i < mFrameBuffers.size(); ++i) {
			// image
			auto img = cgb::image::create2D(
				mSwapChainData->mSwapChainExtent.width, mSwapChainData->mSwapChainExtent.height,
				mSwapChainData->mSwapChainImageFormat.mFormat,
				vk::ImageTiling::eOptimal, 
				vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			mOffscreenImages.emplace_back(std::make_shared<cgb::image>(std::move(img)));
			// view
			mOffscreenImageViews.emplace_back(cgb::image_view::create(mOffscreenImages[i], mOffscreenImages[i]->mInfo.format, vk::ImageAspectFlagBits::eColor));
		}

		// One for each framebuffer like in `void create_descriptor_sets()`, kk?!
		std::vector<vk::DescriptorSetLayout> layouts;
		for (int i = 0; i < mFrameBuffers.size(); ++i) {
			layouts.push_back(mRtDescriptorSetLayout.mDescriptorSetLayout);
		}
		mRtDescriptorSets = cgb::context().create_descriptor_set(layouts);

		for (auto i = 0; i < mRtDescriptorSets.size(); ++i) { // currently, there is only one (see comment above)
			// binding 0:
			auto accStructInfo = vk::WriteDescriptorSetAccelerationStructureNV()
				.setAccelerationStructureCount(1u)
				.setPAccelerationStructures(&mTopLevelAccStructure.mAccStructure);
			auto accStructWrite = vk::WriteDescriptorSet()
				.setPNext(&accStructInfo) // use pNext here instead of pBufferInfo or pImageInfo!
				.setDstSet(mRtDescriptorSets[i].mDescriptorSet)
				.setDstBinding(0u) 
				.setDstArrayElement(0u)
				.setDescriptorCount(1u)
				.setDescriptorType(vk::DescriptorType::eAccelerationStructureNV);

			// binding 1:
			auto outputImageInfo = vk::DescriptorImageInfo()
				.setSampler(nullptr)
				.setImageView(mOffscreenImageViews[i].mImageView) // TODO: can this work? OMG? The plan is to write into the current swap chain's image view (thanks, captain Obvious)
				.setImageLayout(vk::ImageLayout::eGeneral); // TODO: Dunno if that layout is the right choice
			auto outputImageWrite = vk::WriteDescriptorSet()
				.setDstSet(mRtDescriptorSets[i].mDescriptorSet)
				.setDstBinding(1u)
				.setDstArrayElement(0u)
				.setDescriptorCount(1u)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setPImageInfo(&outputImageInfo);

			cgb::context().logical_device().updateDescriptorSets({ accStructWrite, outputImageWrite }, {});
		}
	}

	void initialize() override
	{
		auto rtProps = cgb::context().get_ray_tracing_properties();

		mSwapChainData = cgb::context().get_surf_swap_tuple_for_window(mMainWnd);
		assert(mSwapChainData);

		// create the buffer and its memory
		create_vertex_buffer();
		create_index_buffer();

		// RAY TRACING DATA start
		create_rt_geometry();
		mBottomLevelAccStructure = cgb::acceleration_structure::create_bottom_level(mGeometries);
		create_rt_geometry_instances();
		mTopLevelAccStructure = cgb::acceleration_structure::create_top_level(static_cast<uint32_t>(mGeometryInstances.size()));
		build_acceleration_structures();
		// RAY TRACING DATA end

		load_model();
		create_texture_image();
		mImageView = cgb::image_view::create(mImage, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);
		mSampler = cgb::sampler::create();
		create_depth_buffer();


		// Ordinary graphics pipeline:
		{
			create_descriptor_set_layout();

			auto vert = cgb::shader_handle::create_from_binary_code(cgb::load_binary_file("shader/shader.vert.spv"));
			auto frag = cgb::shader_handle::create_from_binary_code(cgb::load_binary_file("shader/shader.frag.spv"));
			// PROBLEME:
			// - shader_handle* sollte kein Pointer sein!
			std::vector<std::tuple<cgb::shader_type, cgb::shader_handle*>> shaderInfos;
			shaderInfos.push_back(std::make_tuple(cgb::shader_type::vertex, &vert));
			shaderInfos.push_back(std::make_tuple(cgb::shader_type::fragment, &frag));
		

			auto vertexAttribDesc = Vertex::attribute_descriptions();

			mPipeline = cgb::context().create_graphics_pipeline_for_window(
				shaderInfos, 
				mMainWnd, 
				cgb::image_format(mDepthImage->mInfo.format),
				Vertex::binding_description(), 
				vertexAttribDesc.size(), 
				vertexAttribDesc.data(), 
				{ mDescriptorSetLayout.mDescriptorSetLayout });
			mFrameBuffers = cgb::context().create_framebuffers(mPipeline.mRenderPass, mMainWnd, mDepthImageView);
			mCmdBfrs = cgb::context().create_command_buffers_for_graphics(mFrameBuffers.size());
		}

		// Ray tracing pipeline OMG:
		{
			create_rt_descriptor_set_layout();

			auto rgen = cgb::shader_handle::create_from_binary_code(cgb::load_binary_file("shader/rt_basic.rgen.spv"));
			std::vector<std::tuple<cgb::shader_type, cgb::shader_handle*>> shaderInfos;
			shaderInfos.push_back(std::make_tuple(cgb::shader_type::ray_generation, &rgen));

			mRtPipeline = cgb::context().create_ray_tracing_pipeline(shaderInfos, { mRtDescriptorSetLayout.mDescriptorSetLayout });
			mShaderBindingTable = cgb::shader_binding_table::create(mRtPipeline);
		}

		create_uniform_buffers();
		create_descriptor_sets();
		create_rt_descriptor_set();

		for (auto i = 0; i < mCmdBfrs.size(); ++i) { // TODO: WTF, this must be abstracted somehow!
			auto& cmdbfr = mCmdBfrs[i];
			cmdbfr.begin_recording();
			cmdbfr.begin_render_pass(mPipeline.mRenderPass, mFrameBuffers[i].mFramebuffer, { 0, 0 }, mSwapChainData->mSwapChainExtent);
			cmdbfr.mCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipeline.mPipelineLayout, 0u, { mDescriptorSets[i].mDescriptorSet }, {});
			//cgb::context().draw_triangle(mPipeline, cmdbfr);
			//cgb::context().draw_vertices(mPipeline, cmdbfr, mVertexBuffer);
			//cgb::context().draw_indexed(mPipeline, cmdbfr, mVertexBuffer, mIndexBuffer);
			cgb::context().draw_indexed(mPipeline, cmdbfr, mModelVertices, mModelIndices);
			cmdbfr.end_render_pass();

			// TODO: image barriers instead of wait idle!!
			cgb::context().graphics_queue().waitIdle();

			cmdbfr.set_image_barrier(mOffscreenImages[i]->create_barrier(vk::AccessFlags(), vk::AccessFlagBits::eShaderWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral));

			cmdbfr.mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingNV, mRtPipeline.mPipeline);
			cmdbfr.mCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingNV, mRtPipeline.mPipelineLayout, 0u, { mRtDescriptorSets[i].mDescriptorSet }, {});
			cmdbfr.mCommandBuffer.traceRaysNV(
				mShaderBindingTable.mBuffer, 0,
				mShaderBindingTable.mBuffer, 0, rtProps.shaderGroupHandleSize,
				mShaderBindingTable.mBuffer, 0, rtProps.shaderGroupHandleSize,
				nullptr, 0, 0,
				mSwapChainData->mSwapChainExtent.width, mSwapChainData->mSwapChainExtent.height, 1,
				cgb::context().dynamic_dispatch());

			cmdbfr.set_image_barrier(cgb::create_image_barrier(mSwapChainData->mSwapChainImages[i], mSwapChainData->mSwapChainImageFormat.mFormat, vk::AccessFlags(), vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal));
			cmdbfr.set_image_barrier(mOffscreenImages[i]->create_barrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eTransferRead, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal));

			cmdbfr.copy_image(*mOffscreenImages[i], mSwapChainData->mSwapChainImages[i]);

			cmdbfr.set_image_barrier(cgb::create_image_barrier(mSwapChainData->mSwapChainImages[i], mSwapChainData->mSwapChainImageFormat.mFormat, vk::AccessFlagBits::eTransferWrite, vk::AccessFlags(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR));
	

			cmdbfr.end_recording();
		}


		// Add the camera to the composition (and let it handle the updates)
		mQuakeCam.set_position(glm::vec3(-3.0f, 1.0f, 0.0f));
		mQuakeCam.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
		mQuakeCam.SetPerspectiveProjection(glm::radians(60.0f), cgb::current_composition().window_in_focus()->aspect_ratio(), 0.1f, 1000.0f);
		cgb::current_composition().add_element(mQuakeCam);
	}

	void update() override
	{
		if (cgb::input().key_pressed(cgb::key_code::escape)) {
			cgb::current_composition().stop();
		}
	}

	void finalize() override
	{
	}

	void render() override
	{
		uint32_t imageIndex;
		cgb::context().logical_device().acquireNextImageKHR(
			mSwapChainData->mSwapChain, // the swap chain from which we wish to acquire an image [1]
			std::numeric_limits<uint64_t>::max(), // a timeout in nanoseconds for an image to become available. Using the maximum value of a 64 bit unsigned integer disables the timeout. [1]
			cgb::context().image_available_semaphore_current_frame(), // The next two parameters specify synchronization objects that are to be signaled when the presentation engine is finished using the image [1]
			nullptr,
			&imageIndex); // a variable to output the index of the swap chain image that has become available. The index refers to the VkImage in our swapChainImages array. We're going to use that index to pick the right command buffer. [1]

		UniformBufferObject ubo{
			glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * scale(glm::vec3(1.0f)),
			mQuakeCam.CalculateViewMatrix(),
			mQuakeCam.projection_matrix()
			//glm::rotate(glm::mat4(1.0f), cgb::time().frame_time() * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			//glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			//glm::perspective(glm::radians(45.0f), mSwapChainData->mSwapChainExtent.width / static_cast<float>(mSwapChainData->mSwapChainExtent.height), 0.1f, 10.0f)
		};
		// GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. 
		//The easiest way to compensate for that is to flip the sign on the scaling factor of the Y axis in 
		// the projection matrix. If you don't do this, then the image will be rendered upside down. [3]
		ubo.proj[1][1] *= -1;
		mUniformBuffers[imageIndex].fill_host_coherent_memory(&ubo);

		std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		auto submitInfo = vk::SubmitInfo()
			.setWaitSemaphoreCount(1u)
			.setPWaitSemaphores(&cgb::context().image_available_semaphore_current_frame())
			.setPWaitDstStageMask(waitStages.data())
			.setCommandBufferCount(1u)
			.setPCommandBuffers(&mCmdBfrs[imageIndex].mCommandBuffer)
			.setSignalSemaphoreCount(1u)
			.setPSignalSemaphores(&cgb::context().render_finished_semaphore_current_frame());
		// TODO: This only works because we are using cgb::varying_update_only_timer which makes a call to render() in each and every frame
		cgb::context().graphics_queue().submit(1u, &submitInfo, cgb::context().fence_current_frame());

		auto presentInfo = vk::PresentInfoKHR()
			.setWaitSemaphoreCount(1u)
			.setPWaitSemaphores(&cgb::context().render_finished_semaphore_current_frame())
			.setSwapchainCount(1u)
			.setPSwapchains(&mSwapChainData->mSwapChain)
			.setPImageIndices(&imageIndex)
			.setPResults(nullptr);
		cgb::context().presentation_queue().presentKHR(presentInfo);
	}
#endif

private:
	cgb::window* mMainWnd;
	const std::vector<Vertex> mVertices;
	const std::vector<uint16_t> mIndices;
	std::unique_ptr<cgb::Model> mModel;
#ifdef USE_VULKAN_CONTEXT
	cgb::vertex_buffer mModelVertices;
	cgb::index_buffer mModelIndices;
	cgb::vertex_buffer mVertexBuffer;
	cgb::index_buffer mIndexBuffer;
	std::vector<cgb::uniform_buffer> mUniformBuffers;
	cgb::swap_chain_data* mSwapChainData;
	cgb::descriptor_set_layout mDescriptorSetLayout;
	cgb::descriptor_set_layout mRtDescriptorSetLayout;
	cgb::pipeline mPipeline;
	cgb::pipeline mRtPipeline;
	std::vector<cgb::framebuffer> mFrameBuffers;
	std::vector<cgb::command_buffer> mCmdBfrs;
	std::vector<cgb::descriptor_set> mDescriptorSets;
	std::vector<cgb::descriptor_set> mRtDescriptorSets;
	std::shared_ptr<cgb::image> mImage;
	cgb::image_view mImageView;
	cgb::sampler mSampler;
	std::shared_ptr<cgb::image> mDepthImage;
	cgb::image_view mDepthImageView;

	std::vector<vk::GeometryNV> mGeometries;
	std::vector<GeometryInstance> mGeometryInstances;
	std::vector<cgb::buffer> mGeometryInstanceBuffers;
	cgb::acceleration_structure mBottomLevelAccStructure;
	cgb::acceleration_structure mTopLevelAccStructure;
	cgb::shader_binding_table mShaderBindingTable;

	std::vector<std::shared_ptr<cgb::image>> mOffscreenImages;
	std::vector<cgb::image_view> mOffscreenImageViews;
#endif

	cgb::QuakeCamera mQuakeCam;


	// [1] Vulkan Tutorial, Rendering and presentation, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
	// [2] Vulkan Tutorial, Vertex buffer creation, https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
	// [3] Vulkan Tutorial, Descriptor layout and buffer, https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
};


int main()
{
	//try {
		auto selectImageFormat = cgb::context_specific_function<cgb::image_format()>{}
			.SET_VULKAN_FUNCTION([]() { return cgb::image_format(vk::Format::eR8G8B8Unorm); })
			.SET_OPENGL46_FUNCTION([]() { return cgb::image_format{ GL_RGB };  });

		cgb::settings::gApplicationName = "Hello World";
		cgb::settings::gApplicationVersion = cgb::make_version(1, 0, 0);
		cgb::settings::gRequiredDeviceExtensions.push_back("VK_NV_ray_tracing");


		// Create a window which we're going to use to render to
		auto windowParams = cgb::window_params{
			std::nullopt,
			std::nullopt,
			"Hello cg_base World!"
		};
		auto mainWnd = cgb::context().create_window(windowParams, cgb::swap_chain_params{});

		// Create a "behavior" which contains functionality of our program
		auto helloBehavior = hello_behavior(mainWnd);

		// Create a composition of all things that define the essence of 
		// our program, which there are:
		//  - a timer
		//  - an executor
		//  - a window
		//  - a behavior
		auto hello = cgb::composition<cgb::varying_update_only_timer, cgb::sequential_executor>({
				mainWnd 
			}, {
				&helloBehavior
			});

		// Let's go:
		hello.start();
	//}
	//catch (std::runtime_error& re)
	//{
	//	LOG_ERROR_EM(re.what());
	//}
}


