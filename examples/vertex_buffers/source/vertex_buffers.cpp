#include "cg_base.h"

class vertex_buffers_app : public cgb::cg_element
{
	struct Vertex {
	    glm::vec2 pos;
	    glm::vec3 color;

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
			};
			return attribDescs;
		}
	};

	void initialize() override
	{
		const std::vector<Vertex> vertices = {
		    {{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		    {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
		    {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		auto vertexBuffer = cgb::create_and_fill<cgb::vertex_buffer_meta>(
			cgb::vertex_buffer_meta::create_from_data(vertices)
					.describe_member_location(0, &Vertex::pos)
					.describe_member_location(1, &Vertex::color),
			cgb::memory_usage::device,
			vertices.data()
		);

		auto swapChainFormat = cgb::context().main_window()->swap_chain_image_format();
		mPipeline = cgb::graphics_pipeline_for(
			cgb::vertex_input_binding(0, 0, &Vertex::pos),
			cgb::vertex_input_binding(0, 1, &Vertex::color),
			"shaders/passthrough.vert",
			"shaders/color.frag",
			cgb::cfg::front_face::define_front_faces_to_be_clockwise(),
			cgb::cfg::viewport_depth_scissors_config::from_window(cgb::context().main_window()),
			//cgb::renderpass(cgb::renderpass_t::create_good_renderpass((VkFormat)cgb::context().main_window()->swap_chain_image_format().mFormat))
			//std::get<std::shared_ptr<cgb::renderpass_t>>(cgb::context().main_window()->getrenderpass())
			cgb::attachment::create_color(swapChainFormat)
		);

		mCmdBfrs = cgb::context().graphics_queue().pool().get_command_buffers(cgb::context().main_window()->number_of_concurrent_frames(), vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		for (auto i = 0; i < mCmdBfrs.size(); ++i) {
			auto& cmdbfr = mCmdBfrs[i];
			cmdbfr.begin_recording();

			//auto renderPassHandle = cgb::get(mPipeline).renderpass_handle();
			auto renderPassHandle = cgb::context().main_window()->renderpass_handle();
			auto extent = cgb::context().main_window()->swap_chain_extent();

			cmdbfr.begin_render_pass(renderPassHandle, cgb::context().main_window()->backbuffer_at_index(i)->handle(), { 0, 0 }, extent);
			cmdbfr.handle().bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline->handle());
			//cmdbfr.handle().draw(3u, 1u, 0u, 0u);
			//cgb::context().draw_triangle(cgb::get(mPipeline), cmdbfr);
			//cgb::context().draw_vertices(mPipeline, cmdbfr, mVertexBuffer);
			//cgb::context().draw_indexed(mPipeline, cmdbfr, mVertexBuffer, mIndexBuffer);
			//cgb::context().draw_indexed(mPipeline, cmdbfr, mModelVertices, mModelIndices);
			cmdbfr.end_render_pass();

			// TODO: image barriers instead of wait idle!!
			cgb::context().graphics_queue().handle().waitIdle();
			cmdbfr.end_recording();
		}
	}

	void render() override
	{
		auto bufferIndex = cgb::context().main_window()->image_index_for_frame();
		//auto& lol = cgb::context().main_window()->backbuffer_at_index(cgb::context().main_window()->image_index_for_frame(cgb::context().main_window()->current_frame()));
		//LOG_INFO(fmt::format("Current Frame's back buffer id: {}", fmt::ptr(&lol.handle())));
		cgb::context().main_window()->render_frame({ mCmdBfrs[bufferIndex] });
	}

	void update() override
	{
		if (cgb::input().key_pressed(cgb::key_code::h)) {
			// Log a message:
			LOG_INFO_EM("Hello cg_base!");
		}
		if (cgb::input().key_pressed(cgb::key_code::c)) {
			// Center the cursor:
			auto resolution = cgb::context().main_window()->resolution();
			cgb::context().main_window()->set_cursor_pos({ resolution[0] / 2.0, resolution[1] / 2.0 });
		}
		if (cgb::input().key_pressed(cgb::key_code::escape)) {
			// Stop the current composition:
			cgb::current_composition().stop();
		}
	}

private:
	cgb::graphics_pipeline mPipeline;
	std::vector<cgb::command_buffer> mCmdBfrs;
};

int main()
{
	try {
		// What's the name of our application
		cgb::settings::gApplicationName = "Hello, World!";

		// Create a window and open it
		auto mainWnd = cgb::context().create_window("Hello World Window");
		mainWnd->set_resolution({ 640, 480 });
		mainWnd->set_presentaton_mode(cgb::presentation_mode::vsync);
		mainWnd->open(); 

		// Create an instance of vertex_buffers_app which, in this case,
		// contains the entire functionality of our application. 
		auto element = vertex_buffers_app();

		// Create a composition of:
		//  - a timer
		//  - an executor
		//  - a behavior
		// ...
		auto hello = cgb::composition<cgb::varying_update_timer, cgb::sequential_executor>({
				&element
			});

		// ... and start that composition!
		hello.start();
	}
	catch (std::runtime_error& re)
	{
		LOG_ERROR_EM(re.what());
	}
}


