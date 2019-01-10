// hello_world.cpp : Defines the entry point for the console application.
//
#include "cg_base.h"
using namespace std;

class hello_behavior : public cgb::cg_element
{
	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;

		static vk::VertexInputBindingDescription binding_description()
		{
			return vk::VertexInputBindingDescription()
				.setBinding(0u)
				.setStride(sizeof(Vertex))
				.setInputRate(vk::VertexInputRate::eVertex);
		}

		static std::array<vk::VertexInputAttributeDescription, 2> attribute_descriptions()
		{
			return { {
					vk::VertexInputAttributeDescription()
						.setBinding(0u)
						.setLocation(0u)
						.setFormat(vk::Format::eR32G32Sfloat)
						.setOffset(static_cast<uint32_t>(offsetof(Vertex, pos))),
					vk::VertexInputAttributeDescription()
						.setBinding(0u)
						.setLocation(1u)
						.setFormat(vk::Format::eR32G32B32Sfloat)
						.setOffset(static_cast<uint32_t>(offsetof(Vertex, color)))
				} };
		}
	};

	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

public:
	hello_behavior(cgb::window* pMainWnd) 
		: mMainWnd(pMainWnd)
		, mVertices({	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
						{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
						{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
						{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}} })
		, mIndices({ { 0, 1, 2, 2, 3, 0 } })
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
		void* data = cgb::context().logical_device().mapMemory(stagingBuffer.mMemory, 0, mVertexBuffer.mSize);
		memcpy(data, mVertices.data(), stagingBuffer.mSize);
		cgb::context().logical_device().unmapMemory(stagingBuffer.mMemory);

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
		
		void* data = cgb::context().logical_device().mapMemory(stagingBuffer.mMemory, 0, mVertexBuffer.mSize);
		memcpy(data, mIndices.data(), stagingBuffer.mSize);
		cgb::context().logical_device().unmapMemory(stagingBuffer.mMemory);

		mIndexBuffer = cgb::index_buffer::create(
			vk::IndexType::eUint16, mIndices.size(),
			vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

		cgb:copy(stagingBuffer, mIndexBuffer);
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

	auto create_descriptor_set_layout()
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

		return cgb::context().logical_device().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
	}

	void create_descriptor_sets()
	{
		std::vector<vk::DescriptorSetLayout> layouts;
		for (int i = 0; i < mFrameBuffers.size(); ++i) {
			layouts.push_back(create_descriptor_set_layout());
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

			cgb::context().logical_device().updateDescriptorSets({ descriptorWriteBuffer/*, descriptorWriteSampler*/ }, {});
		}
	}

	void create_texture_image()
	{
		int width, height, channels;
		stbi_uc* pixels = stbi_load("assets/texture.jpg", &width, &height, &channels, STBI_rgb_alpha);
		size_t imageSize = width * height * 4;

		if (!pixels) {
			throw std::runtime_error("Couldnt load image using stbi_load");
		}

		auto stagingBuffer = cgb::buffer::create(
			imageSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		// Copy texture into staging buffer
		void* data = cgb::context().logical_device().mapMemory(stagingBuffer.mMemory, 0, stagingBuffer.mSize);
		memcpy(data, pixels, imageSize);
		cgb::context().logical_device().unmapMemory(stagingBuffer.mMemory);

		stbi_image_free(pixels);

		auto img = cgb::image::create2D(width, height);
		cgb::transition_image_layout(img, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		cgb::copy_buffer_to_image(stagingBuffer, img);
		cgb::transition_image_layout(img, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
		mImage = std::make_shared<cgb::image>(std::move(img));
	}

	void initialize() override
	{
		mSwapChainData = cgb::context().get_surf_swap_tuple_for_window(mMainWnd);
		assert(mSwapChainData);

		// create the buffer and its memory
		create_vertex_buffer();
		create_index_buffer();
		create_texture_image();
		mImageView = cgb::image_view::create(mImage, vk::Format::eR8G8B8A8Unorm);
		mSampler = cgb::sampler::create();

		auto vert = cgb::shader_handle::create_from_binary_code(cgb::load_binary_file("shader/shader.vert.spv"));
		auto frag = cgb::shader_handle::create_from_binary_code(cgb::load_binary_file("shader/shader.frag.spv"));
		// PROBLEME:
		// - shader_handle* sollte kein Pointer sein!
		std::vector<std::tuple<cgb::shader_type, cgb::shader_handle*>> shaderInfos;
		shaderInfos.push_back(std::make_tuple(cgb::shader_type::vertex, &vert));
		shaderInfos.push_back(std::make_tuple(cgb::shader_type::fragment, &frag));

		mPipeline = cgb::context().create_graphics_pipeline_for_window(shaderInfos, mMainWnd, Vertex::binding_description(), Vertex::attribute_descriptions(), { create_descriptor_set_layout() });
		mFrameBuffers = cgb::context().create_framebuffers(mPipeline.mRenderPass, mMainWnd);
		mCmdBfrs = cgb::context().create_command_buffers_for_graphics(mFrameBuffers.size());

		create_uniform_buffers();
		create_descriptor_sets();

		for (auto i = 0; i < mCmdBfrs.size(); ++i) { // TODO: WTF, this must be abstracted somehow!
			auto& cmdbfr = mCmdBfrs[i];
			cmdbfr.begin_recording();
			cmdbfr.begin_render_pass(mPipeline.mRenderPass, mFrameBuffers[i].mFramebuffer, { 0, 0 }, mSwapChainData->mSwapChainExtent);
			cmdbfr.mCommandBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				mPipeline.mPipelineLayout,
				0u,
				{ mDescriptorSets[i].mDescriptorSet },
				{});
			//cgb::context().draw_triangle(mPipeline, cmdbfr);
			//cgb::context().draw_vertices(mPipeline, cmdbfr, mVertexBuffer);
			cgb::context().draw_indexed(mPipeline, cmdbfr, mVertexBuffer, mIndexBuffer);
			cmdbfr.end_render_pass();
			cmdbfr.end_recording();
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
			glm::rotate(glm::mat4(1.0f), cgb::time().frame_time() * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::perspective(glm::radians(45.0f), mSwapChainData->mSwapChainExtent.width / static_cast<float>(mSwapChainData->mSwapChainExtent.height), 0.1f, 10.0f)
		};
		// GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. 
		//The easiest way to compensate for that is to flip the sign on the scaling factor of the Y axis in 
		// the projection matrix. If you don't do this, then the image will be rendered upside down. [3]
		ubo.proj[1][1] *= -1;
		void* uboData = cgb::context().logical_device().mapMemory(mUniformBuffers[imageIndex].mMemory, 0, sizeof(ubo));
		memcpy(uboData, &ubo, sizeof(ubo));
		cgb::context().logical_device().unmapMemory(mUniformBuffers[imageIndex].mMemory);

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

	void update() override
	{
		if (cgb::input().key_down(cgb::key_code::a))
			LOG_INFO("a pressed");
		if (cgb::input().key_down(cgb::key_code::s))
			LOG_INFO("s pressed");
		if (cgb::input().key_down(cgb::key_code::w))
			LOG_INFO("w pressed");
		if (cgb::input().key_down(cgb::key_code::d))
			LOG_INFO("d pressed");
		if (cgb::input().key_down(cgb::key_code::escape))
			cgb::current_composition().stop();
	}

private:
	cgb::window* mMainWnd;
	const std::vector<Vertex> mVertices;
	const std::vector<uint16_t> mIndices;
#ifdef USE_VULKAN_CONTEXT
	cgb::vertex_buffer mVertexBuffer;
	cgb::index_buffer mIndexBuffer;
	std::vector<cgb::uniform_buffer> mUniformBuffers;
	cgb::swap_chain_data* mSwapChainData;
	cgb::pipeline mPipeline;
	std::vector<cgb::framebuffer> mFrameBuffers;
	std::vector<cgb::command_buffer> mCmdBfrs;
	std::vector<cgb::descriptor_set> mDescriptorSets;
	std::shared_ptr<cgb::image> mImage;
	cgb::image_view mImageView;
	cgb::sampler mSampler;
#endif

	// [1] Vulkan Tutorial, Rendering and presentation, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
	// [2] Vulkan Tutorial, Vertex buffer creation, https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
	// [3] Vulkan Tutorial, Descriptor layout and buffer, https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
};


int main()
{
	//try {
		auto selectImageFormat = cgb::context_specific_function<cgb::image_format()>{}
			.SET_VULKAN_FUNCTION([]() { return cgb::image_format(vk::Format::eR8G8B8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear); })
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


