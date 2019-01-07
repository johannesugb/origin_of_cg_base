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

public:
	hello_behavior(cgb::window* pMainWnd) 
		: mMainWnd(pMainWnd)
		, mVertices({{ {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} },
					{  {0.5f, 0.5f},  {0.0f, 1.0f, 0.0f} },
					{  {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} }})
	{ 
	}

#ifdef USE_VULKAN_CONTEXT
	void create_vertex_buffer()
	{
		auto bufferInfo = vk::BufferCreateInfo()
			.setSize(static_cast<vk::DeviceSize>(sizeof(mVertices[0]) * mVertices.size()))
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setFlags(vk::BufferCreateFlags()); // The flags parameter is used to configure sparse buffer memory, which is not relevant right now. We'll leave it at the default value of 0. [2]

		mVertexBuffer = cgb::context().logical_device().createBuffer(bufferInfo);
	}

	void delete_vertex_buffer()
	{
		cgb::context().logical_device().destroyBuffer(mVertexBuffer);
	}

	void initialize() override
	{
		mSwapChainData = cgb::context().get_surf_swap_tuple_for_window(mMainWnd);
		assert(mSwapChainData);

		create_vertex_buffer();

		auto vert = cgb::shader_handle::create_from_binary_code(cgb::load_binary_file("shader/shader.vert.spv"));
		auto frag = cgb::shader_handle::create_from_binary_code(cgb::load_binary_file("shader/shader.frag.spv"));
		// PROBLEME:
		// - shader_handle* sollte kein Pointer sein!
		std::vector<std::tuple<cgb::shader_type, cgb::shader_handle*>> shaderInfos;
		shaderInfos.push_back(std::make_tuple(cgb::shader_type::vertex, &vert));
		shaderInfos.push_back(std::make_tuple(cgb::shader_type::fragment, &frag));
		mPipeline = cgb::context().create_graphics_pipeline_for_window(shaderInfos, mMainWnd, Vertex::binding_description(), Vertex::attribute_descriptions());
		mFrameBuffers = cgb::context().create_framebuffers(mPipeline.mRenderPass, mMainWnd);
		mCmdPool = cgb::context().create_command_pool();
		mCmdBfrs = cgb::context().create_command_buffers(static_cast<uint32_t>(mFrameBuffers.size()), mCmdPool);
		for (auto i = 0; i < mCmdBfrs.size(); ++i) { // TODO: WTF, this must be abstracted somehow!
			auto& cmdbfr = mCmdBfrs[i];
			cmdbfr.begin_recording();
			cmdbfr.begin_render_pass(mPipeline.mRenderPass, mFrameBuffers[i].mFramebuffer, { 0, 0 }, mSwapChainData->mSwapChainExtent);
			cgb::context().draw_triangle(mPipeline, cmdbfr);
			cmdbfr.end_render_pass();
			cmdbfr.end_recording();
		}
	}

	void finalize() override
	{
		delete_vertex_buffer();
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

		auto submitInfo = vk::SubmitInfo()
			.setWaitSemaphoreCount(1u)
			.setPWaitSemaphores(&cgb::context().image_available_semaphore_current_frame())
			.setPWaitDstStageMask(std::begin({ vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput) }))
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
#ifdef USE_VULKAN_CONTEXT
	vk::Buffer mVertexBuffer;
	cgb::swap_chain_data* mSwapChainData;
	cgb::pipeline mPipeline;
	std::vector<cgb::framebuffer> mFrameBuffers;
	cgb::command_pool mCmdPool;
	std::vector<cgb::command_buffer> mCmdBfrs;
#endif

	// [1] Vulkan Tutorial, Rendering and presentation, https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
	// [2] Vulkan Tutorial, Vertex buffer creation, https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
};


int main()
{
	try {
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
	}
	catch (std::runtime_error& re)
	{
		LOG_ERROR_EM(re.what());
	}
}


