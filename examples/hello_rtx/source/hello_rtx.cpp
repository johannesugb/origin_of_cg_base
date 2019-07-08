// hello_world.cpp : Defines the entry point for the console application.
//
#include "cg_base.h"
#include "temp.h"
using namespace std;

class my_first_rtx_app : public cgb::cg_element
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

	struct VkGeometryInstance
	{
		float transform[12];
		uint32_t instanceId : 24;
		uint32_t mask : 8;
		uint32_t instanceOffset : 24;
		uint32_t flags : 8;
		uint64_t accelerationStructureHandle;
	};

public:
	my_first_rtx_app() 
		: mVertices({	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
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
		mVertexBuffer = cgb::create_and_fill(
			cgb::vertex_buffer_data{ sizeof(mVertices[0]), mVertices.size() },
			cgb::memory_usage::device,
			mVertices.data());

		// Mind the following:
		//		void vulkan::set_sharing_mode_for_transfer(vk::BufferCreateInfo& pCreateInfo)
		//		{

		//			// TODO: OMG, not only compare the QUEUE INDICES, but also the QUEUE FAMILY INDICES ffs (oder evtl. sogar NUR die QUEUE FAMILY INDICES?)

		//			if (graphics_queue_index() == transfer_queue_index()) {
		//				pCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
		//			}
		//			else {
		//				pCreateInfo.setSharingMode(vk::SharingMode::eConcurrent);
		//				pCreateInfo.setQueueFamilyIndexCount(static_cast<uint32_t>(mAllUsedQueueFamilyIndices.size()));
		//				pCreateInfo.setPQueueFamilyIndices(mAllUsedQueueFamilyIndices.data());
		//			}
		//		}
	}

	void create_index_buffer()
	{
		mIndexBuffer = cgb::create_and_fill(
			cgb::index_buffer_data{ sizeof(mIndices[0]), mIndices.size() },
			cgb::memory_usage::device,
			mIndices.data());
	}

	static void load_model(std::string inPath, std::unique_ptr<cgb::model_data>& outModel, cgb::vertex_buffer_t& outVertexBuffer, cgb::index_buffer_t& outIndexBuffer, int mesh_index)
	{
		outModel = cgb::model_data::LoadFromFile(inPath, glm::mat4(1.0f));
		auto& mesh = outModel->mesh_at(mesh_index);
		
		{
			outVertexBuffer = cgb::create_and_fill(
				cgb::vertex_buffer_data{ mesh.m_size_one_vertex, mesh.m_vertex_data.size() / mesh.m_size_one_vertex },
				cgb::memory_usage::device,
				mesh.m_vertex_data.data());
		}

		{
			outIndexBuffer = cgb::create_and_fill(
				cgb::index_buffer_data{ sizeof(mesh.m_indices[0]), mesh.m_indices.size() },
				cgb::memory_usage::device,
				mesh.m_indices.data());
		}
	}

	void create_uniform_buffers()
	{
		for (auto i = 0; i < mFrameBuffers.size(); ++i) {
			mUniformBuffers.push_back(
				cgb::create(
					cgb::uniform_buffer_data{ sizeof(UniformBufferObject) },
					cgb::memory_usage::host_coherent
				));

		}
	}

	void create_rt_uniform_buffers()
	{
		for (auto i = 0; i < mFrameBuffers.size(); ++i) {
			auto& vec = mRtUniformBuffers.emplace_back();
			vec.emplace_back(cgb::create(
				cgb::uniform_buffer_data{ sizeof(UniformBufferObject) },
				cgb::memory_usage::host_coherent, vk::BufferUsageFlagBits::eRayTracingNV
			));
			vec.emplace_back(cgb::create( // another 2 for color values
				cgb::uniform_buffer_data{ sizeof(glm::vec4) },
				cgb::memory_usage::host_coherent, vk::BufferUsageFlagBits::eRayTracingNV
			));
			vec.emplace_back(cgb::create( // another 2 for color values
				cgb::uniform_buffer_data{ sizeof(glm::vec4) },
				cgb::memory_usage::host_coherent, vk::BufferUsageFlagBits::eRayTracingNV
			));
		}
	}

	void create_descriptor_set_layout()
	{
		mDescriptorSetLayout = cgb::layout_for({
			cgb::binding(0, cgb::get<cgb::uniform_buffer_t>(mUniformBuffers[0])),
			cgb::binding(1, mImageView)
		});
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
			// Achtung: Wichtig, hier alle Flags zu setzen, für welche Shader das Ding genutzt werden soll... d.h. ich will RayGen und aber auch ClosestHit, z.B.:
			.setStageFlags(vk::ShaderStageFlagBits::eRaygenNV | vk::ShaderStageFlagBits::eClosestHitNV)
			.setPImmutableSamplers(nullptr)
			,
			// Output Image Layout Binding:
			vk::DescriptorSetLayoutBinding()
			.setBinding(1u)
			.setDescriptorType(vk::DescriptorType::eStorageImage)
			.setDescriptorCount(1u)
			.setStageFlags(vk::ShaderStageFlagBits::eRaygenNV) // ...warum hier allerdings nur RayGen bleibt - k.A.
			.setPImmutableSamplers(nullptr)
			,
			// UBO Binding
			vk::DescriptorSetLayoutBinding()
			.setBinding(2u)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1u)
			.setStageFlags(vk::ShaderStageFlagBits::eAll)
			.setPImmutableSamplers(nullptr) // The pImmutableSamplers field is only relevant for image sampling related descriptors [3]
			,
			// UBO Binding (instance data)
			vk::DescriptorSetLayoutBinding()
			.setBinding(3u)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(2u) // _instanceNum is an upper bound
			.setStageFlags(vk::ShaderStageFlagBits::eAll)
			.setPImmutableSamplers(nullptr) // The pImmutableSamplers field is only relevant for image sampling related descriptors [3]
		};

		auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setFlags(vk::DescriptorSetLayoutCreateFlags())
			.setBindingCount(static_cast<uint32_t>(descriptorSetLayoutBindings.size()))
			.setPBindings(descriptorSetLayoutBindings.data());

		mRtDescriptorSetLayout = cgb::descriptor_set_layout::create(descriptorSetLayoutCreateInfo);

		mRtDescriptorSetLayout = cgb::layout_for({
			cgb::binding(0, mTopLevelAccStructure),
			cgb::binding(1, mImageView)
			});
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
				.setBuffer(mUniformBuffers[i].buffer_handle())
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

		auto stagingBuffer = cgb::create_and_fill(
			cgb::generic_buffer_data{ imageSize },
			cgb::memory_usage::host_coherent,
			pixels,
			nullptr,
			vk::BufferUsageFlagBits::eTransferSrc);
			
		stbi_image_free(pixels);

		auto img = cgb::image_t::create(width, height, cgb::image_format(vk::Format::eR8G8B8A8Unorm), cgb::memory_usage::device);
		cgb::transition_image_layout(img, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		cgb::copy_buffer_to_image(stagingBuffer, img);
		cgb::transition_image_layout(img, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
		mImage = cgb::make_shared(img);

		cgb::context().logical_device().waitIdle();
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

		auto extent = cgb::context().main_window()->swap_chain_extent();
		mDepthImage = std::make_shared<cgb::image_t>(std::move(cgb::image_t::create2D(
			extent.width, extent.height,
			selectedFormat,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal)));
		mDepthImageView = cgb::image_view::create(mDepthImage, selectedFormat, vk::ImageAspectFlagBits::eDepth);
		cgb::transition_image_layout(*mDepthImage, selectedFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	}

	void create_rt_geometry()
	{
		// chalet:
		mGeometries.emplace_back()
			.setGeometryType(vk::GeometryTypeNV::eTriangles)
			.setGeometry(vk::GeometryDataNV()
						 .setTriangles(vk::GeometryTrianglesNV()
									   .setVertexData(mModelVertices.buffer_handle())
									   .setVertexOffset(0)
									   .setVertexCount(mModelVertices.config().num_elements())
									   .setVertexStride(mModelVertices.config().sizeof_one_element())
									   .setVertexFormat(vk::Format::eR32G32B32Sfloat)
									   .setIndexData(mModelIndices.buffer_handle())
									   .setIndexOffset(0)
									   .setIndexCount(mModelIndices.config().num_elements())
									   .setIndexType(cgb::convert_to_vk_index_type(mModelIndices.config().sizeof_one_element()))
									   .setTransformData(nullptr)
									   .setTransformOffset(0)))
			.setFlags(vk::GeometryFlagBitsNV::eOpaque); 
		//// sphere:
		//mGeometries.emplace_back()
		//	.setGeometryType(vk::GeometryTypeNV::eTriangles)
		//	.setGeometry(vk::GeometryDataNV()
		//				 .setTriangles(vk::GeometryTrianglesNV()
		//							   .setVertexData(mSphereVertices.mBuffer)
		//							   .setVertexOffset(0)
		//							   .setVertexCount(mSphereVertices.mVertexCount)
		//							   .setVertexStride(mSphereVertices.mSize / mSphereVertices.mVertexCount)
		//							   .setVertexFormat(vk::Format::eR32G32B32Sfloat)
		//							   .setIndexData(mSphereIndices.mBuffer)
		//							   .setIndexOffset(0)
		//							   .setIndexCount(mSphereIndices.mIndexCount)
		//							   .setIndexType(mSphereIndices.mIndexType)
		//							   .setTransformData(nullptr)
		//							   .setTransformOffset(0)));
	}

	void create_rt_geometry_instances()
	{
		auto instId = 0u;
		// 1st instance of chalet
		{
			auto modelMatrixForInstance = glm::transpose(glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * scale(glm::vec3(1.0f)));
			VkGeometryInstance inst;
			memcpy(inst.transform, glm::value_ptr(modelMatrixForInstance), sizeof(inst.transform));
			inst.instanceId = instId++;
			inst.mask = 0xff;
			inst.instanceOffset = 0;
			inst.flags = static_cast<uint32_t>(vk::GeometryInstanceFlagBitsNV::eTriangleCullDisable);
			inst.accelerationStructureHandle = mBottomLevelAccStructure.mHandle.mHandle;
			mGeometryInstances.push_back(inst);
		}
		// 2nd instance of chalet
		{
			auto modelMatrixForInstance = glm::transpose(glm::translate(glm::vec3(0.0f, 0.0f, -3.0f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * scale(glm::vec3(1.0f)));
			VkGeometryInstance inst;
			memcpy(inst.transform, glm::value_ptr(modelMatrixForInstance), sizeof(inst.transform));
			inst.instanceId = instId++;
			inst.mask = 0xff;
			inst.instanceOffset = 0;
			inst.flags = static_cast<uint32_t>(vk::GeometryInstanceFlagBitsNV::eTriangleCullDisable);
			inst.accelerationStructureHandle = mBottomLevelAccStructure.mHandle.mHandle;
			mGeometryInstances.push_back(inst);
		}

		mGeometryInstanceBuffer = cgb::create_and_fill(
			cgb::generic_buffer_data{ sizeof(VkGeometryInstance) * mGeometryInstances.size() },
			cgb::memory_usage::host_coherent, 
			mGeometryInstances.data(),
			nullptr, // There will be no semaphore because it is host coherent
			vk::BufferUsageFlagBits::eRayTracingNV
		);
	}

	void build_acceleration_structures()
	{
		auto scratchBuffer = cgb::create(
			cgb::generic_buffer_data{ std::max(mBottomLevelAccStructure.get_scratch_buffer_size(), mTopLevelAccStructure.get_scratch_buffer_size()) },
			cgb::memory_usage::device,
			vk::BufferUsageFlagBits::eRayTracingNV
		);

		auto cmdBfr = cgb::context().graphics_queue().pool().get_command_buffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdBfr.begin_recording();

		auto memoryBarrier = vk::MemoryBarrier()
			.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteNV | vk::AccessFlagBits::eAccelerationStructureReadNV)
			.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteNV | vk::AccessFlagBits::eAccelerationStructureReadNV);

		// Build BLAS
		cmdBfr.handle().buildAccelerationStructureNV(
			mBottomLevelAccStructure.mAccStructureInfo,
			nullptr, 0,								// no instance data for bottom level AS
			VK_FALSE,								// update = false
			mBottomLevelAccStructure.mAccStructure, // destination AS
			nullptr,								// no source AS
			scratchBuffer.buffer_handle(), 0,		// scratch buffer + offset
			cgb::context().dynamic_dispatch());

		// Barrier
		cmdBfr.handle().pipelineBarrier(
			vk::PipelineStageFlagBits::eAccelerationStructureBuildNV,
			vk::PipelineStageFlagBits::eAccelerationStructureBuildNV,
			vk::DependencyFlags(),
			{ memoryBarrier }, 
			{}, {});

		// Build TLAS
		cmdBfr.handle().buildAccelerationStructureNV(
			mTopLevelAccStructure.mAccStructureInfo,
			mGeometryInstanceBuffer.buffer_handle(), 0,	// buffer containing the instance data (only one)
			VK_FALSE,									// update = false
			mTopLevelAccStructure.mAccStructure,		// destination AS
			nullptr,									// no source AS
			scratchBuffer.buffer_handle(), 0,			// scratch buffer + offset
			cgb::context().dynamic_dispatch());

		// Barrier
		cmdBfr.handle().pipelineBarrier(
			vk::PipelineStageFlagBits::eAccelerationStructureBuildNV,
			vk::PipelineStageFlagBits::eRayTracingShaderNV,
			vk::DependencyFlags(),
			{ memoryBarrier },
			{}, {});

		cmdBfr.end_recording();
		auto submitInfo = vk::SubmitInfo()
			.setCommandBufferCount(1u)
			.setPCommandBuffers(cmdBfr.handle_addr());
		cgb::context().graphics_queue().handle().submit({ submitInfo }, nullptr); 
		cgb::context().graphics_queue().handle().waitIdle();
	}

	void create_rt_descriptor_set()
	{
		auto extent = cgb::context().main_window()->swap_chain_extent();
		auto format = cgb::context().main_window()->swap_chain_image_format();

		// create an offscreen image for each one:
		for (int i = 0; i < mFrameBuffers.size(); ++i) {
			// image
			auto img = cgb::image_t::create2D(
				extent.width, extent.height,
				format.mFormat,
				vk::ImageTiling::eOptimal, 
				vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			mOffscreenImages.emplace_back(std::make_shared<cgb::image_t>(std::move(img)));
			// view
			mOffscreenImageViews.emplace_back(cgb::image_view::create(mOffscreenImages[i], mOffscreenImages[i]->mInfo.format, vk::ImageAspectFlagBits::eColor));
		}

		// One for each framebuffer like in `void create_descriptor_sets()`, kk?!
		std::vector<vk::DescriptorSetLayout> layouts;
		for (int i = 0; i < mFrameBuffers.size(); ++i) {
			layouts.push_back(mRtDescriptorSetLayout.mDescriptorSetLayout);
		}
		mRtDescriptorSets = cgb::context().create_descriptor_set(layouts);

		for (auto i = 0; i < mRtDescriptorSets.size(); ++i) {
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

			// binding 2:
			auto bufferInfo = vk::DescriptorBufferInfo()
				.setBuffer(mRtUniformBuffers[i][0].buffer_handle())
				.setOffset(0)
				.setRange(sizeof(UniformBufferObject));
			auto descriptorWriteBuffer = vk::WriteDescriptorSet()
				.setDstSet(mRtDescriptorSets[i].mDescriptorSet) // Note: Always pay attention to reference the right descriptor set!
				.setDstBinding(2u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1u)
				.setPBufferInfo(&bufferInfo);

			// binding 3:
			std::array bufferDescs = {
				vk::DescriptorBufferInfo()
				.setBuffer(mRtUniformBuffers[i][1].buffer_handle())
				.setOffset(0)
				.setRange(sizeof(UniformBufferObject))
				,
				vk::DescriptorBufferInfo()
				.setBuffer(mRtUniformBuffers[i][2].buffer_handle())
				.setOffset(0)
				.setRange(sizeof(UniformBufferObject))
			};
			auto descriptorWriteBuffer2 = vk::WriteDescriptorSet()
				.setDstSet(mRtDescriptorSets[i].mDescriptorSet) // Note: Always pay attention to reference the right descriptor set!
				.setDstBinding(3u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(static_cast<uint32_t>(bufferDescs.size()))
				.setPBufferInfo(bufferDescs.data());

			cgb::context().logical_device().updateDescriptorSets({ accStructWrite, outputImageWrite, descriptorWriteBuffer, descriptorWriteBuffer2 }, {}); // ! Never forgetti!
		}
	}

	void initialize() override
	{
		// temp:
		ResourceBase::Init(
			(VkPhysicalDevice)cgb::context().physical_device(),
			(VkDevice)cgb::context().logical_device(),
			(VkCommandPool)cgb::context().transfer_queue().pool().handle(),
			(VkQueue)cgb::context().transfer_queue().handle());

		auto rtProps = cgb::context().get_ray_tracing_properties();

		// create the buffer and its memory
		create_vertex_buffer();
		cgb::context().logical_device().waitIdle();
		create_index_buffer();
		cgb::context().logical_device().waitIdle();

		load_model("assets/sponza_structure.obj", mModel, mModelVertices, mModelIndices, 2);
		cgb::context().logical_device().waitIdle();
		load_model("assets/sphere.obj", mSphere, mSphereVertices, mSphereIndices, 0);
		cgb::context().logical_device().waitIdle();
		create_texture_image();
		mImageView = cgb::image_view::create(mImage, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);
		mSampler = cgb::sampler::create();
		create_depth_buffer();

		//// RAY TRACING DATA start
		//create_rt_geometry();
		//mBottomLevelAccStructure = cgb::acceleration_structure::create_bottom_level(mGeometries);
		//create_rt_geometry_instances();
		//mTopLevelAccStructure = cgb::acceleration_structure::create_top_level(static_cast<uint32_t>(mGeometryInstances.size()));
		//build_acceleration_structures();
		//// RAY TRACING DATA end

		// Ordinary graphics pipeline:
		{
			create_descriptor_set_layout();

			auto vert = cgb::shader::create_from_binary_code(cgb::load_binary_file("shaders/shader.vert.spv"));
			auto frag = cgb::shader::create_from_binary_code(cgb::load_binary_file("shaders/shader.frag.spv"));
			// PROBLEME:
			// - shader_handle* sollte kein Pointer sein!
			std::vector<std::tuple<cgb::shader_type, cgb::shader*>> shaderInfos;
			shaderInfos.push_back(std::make_tuple(cgb::shader_type::vertex, &vert));
			shaderInfos.push_back(std::make_tuple(cgb::shader_type::fragment, &frag));
		

			auto vertexAttribDesc = Vertex::attribute_descriptions();

			mPipeline = cgb::context().create_graphics_pipeline_for_window(
				shaderInfos, 
				cgb::context().main_window(), 
				cgb::image_format(mDepthImage->mInfo.format),
				Vertex::binding_description(), 
				vertexAttribDesc.size(), 
				vertexAttribDesc.data(), 
				{ mDescriptorSetLayout.mDescriptorSetLayout });
			mFrameBuffers = cgb::context().create_framebuffers(mPipeline.mRenderPass, cgb::context().main_window(), mDepthImageView);
			mCmdBfrs = cgb::context().graphics_queue().pool().get_command_buffers(mFrameBuffers.size());
		}

		//// Ray tracing pipeline OMG:
		//{
		//	create_rt_descriptor_set_layout();

		//	auto rgen = cgb::shader::create_from_binary_code(cgb::load_binary_file("shaders/rt_09_first.rgen.spv"));
		//	auto rchit = cgb::shader::create_from_binary_code(cgb::load_binary_file("shaders/rt_09_first.rchit.spv"));
		//	auto rmiss = cgb::shader::create_from_binary_code(cgb::load_binary_file("shaders/rt_09_first.rmiss.spv"));
		//	auto rchit2 = cgb::shader::create_from_binary_code(cgb::load_binary_file("shaders/rt_09_secondary.rchit.spv"));
		//	auto rmiss2 = cgb::shader::create_from_binary_code(cgb::load_binary_file("shaders/rt_09_secondary.rmiss.spv"));
		//	std::vector<std::tuple<cgb::shader_type, cgb::shader*>> shaderInfos;
		//	shaderInfos.push_back(std::make_tuple(cgb::shader_type::ray_generation, &rgen));
		//	shaderInfos.push_back(std::make_tuple(cgb::shader_type::closest_hit, &rchit));
		//	shaderInfos.push_back(std::make_tuple(cgb::shader_type::closest_hit, &rchit2));
		//	shaderInfos.push_back(std::make_tuple(cgb::shader_type::miss, &rmiss));
		//	shaderInfos.push_back(std::make_tuple(cgb::shader_type::miss, &rmiss2));

		//	mRtPipeline = cgb::context().create_ray_tracing_pipeline(shaderInfos, { mRtDescriptorSetLayout.mDescriptorSetLayout });
		//	mShaderBindingTable = cgb::shader_binding_table::create(mRtPipeline);
		//}

		create_uniform_buffers();
		//create_rt_uniform_buffers();
		create_descriptor_sets();
		//create_rt_descriptor_set();

		auto extent = cgb::context().main_window()->swap_chain_extent();

		for (auto i = 0; i < mCmdBfrs.size(); ++i) { // TODO: WTF, this must be abstracted somehow!
			auto& cmdbfr = mCmdBfrs[i];
			cmdbfr.begin_recording();
			cmdbfr.begin_render_pass(mPipeline.mRenderPass, mFrameBuffers[i].mFramebuffer, { 0, 0 }, extent);
			cmdbfr.handle().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipeline.mPipelineLayout, 0u, { mDescriptorSets[i].mDescriptorSet }, {});
			//cgb::context().draw_triangle(mPipeline, cmdbfr);
			//cgb::context().draw_vertices(mPipeline, cmdbfr, mVertexBuffer);
			//cgb::context().draw_indexed(mPipeline, cmdbfr, mVertexBuffer, mIndexBuffer);
			cgb::context().draw_indexed(mPipeline, cmdbfr, mModelVertices, mModelIndices);
			cmdbfr.end_render_pass();

			// TODO: image barriers instead of wait idle!!
			cgb::context().graphics_queue().handle().waitIdle();

			//cmdbfr.set_image_barrier(mOffscreenImages[i]->create_barrier(vk::AccessFlags(), vk::AccessFlagBits::eShaderWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral));

			//cmdbfr.handle().bindPipeline(vk::PipelineBindPoint::eRayTracingNV, mRtPipeline.mPipeline);
			//cmdbfr.handle().bindDescriptorSets(vk::PipelineBindPoint::eRayTracingNV, mRtPipeline.mPipelineLayout, 0u, { mRtDescriptorSets[i].mDescriptorSet }, {});
			//cmdbfr.handle().traceRaysNV(
			//	mShaderBindingTable.mBuffer, 0,
			//	mShaderBindingTable.mBuffer, 3 * rtProps.shaderGroupHandleSize, rtProps.shaderGroupHandleSize,
			//	mShaderBindingTable.mBuffer, 1 * rtProps.shaderGroupHandleSize, rtProps.shaderGroupHandleSize,
			//	nullptr, 0, 0,
			//	extent.width, extent.height, 1,
			//	cgb::context().dynamic_dispatch());

			//cmdbfr.set_image_barrier(cgb::create_image_barrier(mSwapChainData->mSwapChainImages[i], mSwapChainData->mSwapChainImageFormat.mFormat, vk::AccessFlags(), vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal));
			//cmdbfr.set_image_barrier(mOffscreenImages[i]->create_barrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eTransferRead, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal));

			//cmdbfr.copy_image(*mOffscreenImages[i], mSwapChainData->mSwapChainImages[i]);

			//cmdbfr.set_image_barrier(cgb::create_image_barrier(mSwapChainData->mSwapChainImages[i], mSwapChainData->mSwapChainImageFormat.mFormat, vk::AccessFlagBits::eTransferWrite, vk::AccessFlags(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR));
	

			cmdbfr.end_recording();
		}


		// Add the camera to the composition (and let it handle the updates)
		mQuakeCam.set_translation({ 0.0f, 0.0f, 0.0f });
		mQuakeCam.set_perspective_projection(glm::radians(60.0f), cgb::context().main_window()->aspect_ratio(), 0.5f, 100.0f);
		//mQuakeCam.set_orthographic_projection(-5, 5, -5, 5, 0.5, 100);
		cgb::current_composition().add_element(mQuakeCam);
	}

	void update() override
	{
		if (cgb::input().key_pressed(cgb::key_code::escape)) {
			cgb::current_composition().stop();
		}
		if (cgb::input().key_pressed(cgb::key_code::c)) {
			cgb::context().main_window()->set_cursor_pos({ 666.0, 100 });
		}
		if (cgb::input().key_pressed(cgb::key_code::tab)) {
			if (mQuakeCam.is_enabled()) {
				mQuakeCam.disable();
			}
			else {
				mQuakeCam.enable();
			}
		}
	}

	void finalize() override
	{
	}

	void render() override
	{
		auto* wnd = cgb::context().main_window();

		UniformBufferObject ubo{
			//glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * scale(glm::vec3(1.0f)),
			glm::scale(glm::vec3(0.01f)),
			mQuakeCam.view_matrix(),
			mQuakeCam.projection_matrix()
			//glm::rotate(glm::mat4(1.0f), cgb::time().frame_time() * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			//glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			//glm::perspective(glm::radians(45.0f), mSwapChainData->mSwapChainExtent.width / static_cast<float>(mSwapChainData->mSwapChainExtent.height), 0.1f, 10.0f)
		};
		// GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. 
		//The easiest way to compensate for that is to flip the sign on the scaling factor of the Y axis in 
		// the projection matrix. If you don't do this, then the image will be rendered upside down. [3]
		//ubo.proj[1][1] *= -1;
		auto bufferIndex = wnd->image_index_for_frame();
		cgb::fill(mUniformBuffers[bufferIndex], &ubo);
		//cgb::fill(mRtUniformBuffers[bufferIndex][0], &ubo);
		//glm::vec4 color1(1.0, 1.0, 0.0, 0.0);
		//glm::vec4 color2(1.0, 0.0, 0.0, 0.0);
		//cgb::fill(mRtUniformBuffers[bufferIndex][1], &color1);
		//cgb::fill(mRtUniformBuffers[bufferIndex][2], &color2);

		wnd->render_frame({ mCmdBfrs[bufferIndex] });
	}
#endif

private:
	// Raw-Data, kk:
	const std::vector<Vertex> mVertices;
	const std::vector<uint16_t> mIndices;

	cgb::vertex_buffer_t mVertexBuffer;
	cgb::index_buffer_t mIndexBuffer;

	// Ab hier wird's ugly:
	std::unique_ptr<cgb::model_data> mModel;
	std::unique_ptr<cgb::model_data> mSphere;
	std::vector<cgb::uniform_buffer_t> mUniformBuffers;
	std::vector<std::vector<cgb::uniform_buffer_t>> mRtUniformBuffers;
	vk::UniqueDescriptorSetLayout mDescriptorSetLayout;
	vk::UniqueDescriptorSetLayout mRtDescriptorSetLayout;
	image mImage;
	cgb::vertex_buffer_t mModelVertices;
	cgb::index_buffer_t mModelIndices;
	cgb::vertex_buffer_t mSphereVertices;
	cgb::index_buffer_t mSphereIndices;
	cgb::generic_buffer_t mGeometryInstanceBuffer;


	cgb::pipeline mPipeline;
	cgb::pipeline mRtPipeline;
	std::vector<cgb::framebuffer> mFrameBuffers;
	std::vector<cgb::command_buffer> mCmdBfrs;
	std::vector<cgb::descriptor_set> mDescriptorSets;
	std::vector<cgb::descriptor_set> mRtDescriptorSets;
	cgb::image_view mImageView;
	cgb::sampler mSampler;
	std::shared_ptr<cgb::image_t> mDepthImage;
	cgb::image_view mDepthImageView;

	std::vector<vk::GeometryNV> mGeometries;
	std::vector<VkGeometryInstance> mGeometryInstances;
	cgb::acceleration_structure mBottomLevelAccStructure;
	cgb::acceleration_structure mTopLevelAccStructure;
	cgb::shader_binding_table mShaderBindingTable;

	std::vector<std::shared_ptr<cgb::image_t>> mOffscreenImages;
	std::vector<cgb::image_view> mOffscreenImageViews;



	cgb::quake_camera mQuakeCam;


	// [1] Vulkan Tutorial, Rendering and presentation, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
	// [2] Vulkan Tutorial, Vertex buffer creation, https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
	// [3] Vulkan Tutorial, Descriptor layout and buffer, https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
};


int main()
{
	try {
		cgb::settings::gApplicationName = "Hello World";
		cgb::settings::gApplicationVersion = cgb::make_version(1, 0, 0);
		cgb::settings::gRequiredDeviceExtensions.push_back("VK_NV_ray_tracing");
		cgb::settings::gRequiredDeviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);


		// Create a window which we're going to use to render to
		auto mainWnd = cgb::context().create_window("Hello RTX!");
		mainWnd->set_resolution({ 1600, 900 });
		mainWnd->set_presentaton_mode(cgb::presentation_mode::vsync);
		mainWnd->open();

		// Create a "behavior" which contains functionality of our program
		auto behavior = my_first_rtx_app();

		// Create a composition of all things that define the essence of 
		// our program, which there are:
		//  - a timer
		//  - an executor
		//  - a window
		//  - a behavior
		auto hello = cgb::composition<cgb::varying_update_timer, cgb::sequential_executor>({
				&behavior
			});

		// Let's go:
		hello.start();
	}
	catch (std::runtime_error& re)
	{
		LOG_ERROR_EM(re.what());
	}
}


