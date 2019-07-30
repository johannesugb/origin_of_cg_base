#include "cg_base.h"

class my_first_cgb_app : public cgb::cg_element
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

	void initialize() override
	{
		auto swapChainFormat = cgb::context().main_window()->swap_chain_image_format();
		static auto pipe = cgb::graphics_pipeline_for(
			"shaders/a_triangle.vert",
			"shaders/a_triangle.frag",
			cgb::viewport_depth_scissors_config::from_window(cgb::context().main_window()),
			cgb::attachment::create_color(swapChainFormat, true)
		);

		mCmdBfrs = cgb::context().graphics_queue().pool().get_command_buffers(cgb::context().main_window()->number_of_concurrent_frames());
		for (auto i = 0; i < mCmdBfrs.size(); ++i) { // TODO: WTF, this must be abstracted somehow!
			auto& cmdbfr = mCmdBfrs[i];
			cmdbfr.begin_recording();
			cmdbfr.begin_render_pass(pipe.renderpass_handle(), cgb::context().main_window()->backbuffer_at_index(i).handle(), { 0, 0 }, cgb::context().main_window()->swap_chain_extent());
			//cmdbfr.handle().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipe.layout_handle(), 0u, { }, {});
			cgb::context().draw_triangle(pipe, cmdbfr);
			//cgb::context().draw_vertices(mPipeline, cmdbfr, mVertexBuffer);
			//cgb::context().draw_indexed(mPipeline, cmdbfr, mVertexBuffer, mIndexBuffer);
			//cgb::context().draw_indexed(mPipeline, cmdbfr, mModelVertices, mModelIndices);
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
	cgb::graphics_pipeline_t mPipeline;
	std::vector<cgb::command_buffer> mCmdBfrs;
};

int main()
{
	try {
		// What's the name of our application
		cgb::settings::gApplicationName = "Hello, World!";

		// Create a window and open it
		auto mainWnd = cgb::context().create_window("Hello World Window");
		mainWnd->set_resolution({ 320, 200 });
		mainWnd->set_presentaton_mode(cgb::presentation_mode::vsync);
		mainWnd->open(); 

		// Create an instance of my_first_cgb_app which, in this case,
		// contains the entire functionality of our application. 
		auto element = my_first_cgb_app();

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


