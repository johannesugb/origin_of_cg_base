#include "cg_base.h"

class hello_behavior : public cgb::cg_element
{
	//void initialize() override
	//{
	//	cgb::context().create_sync_objects(); // <-- TODO
	//	auto swapChain = cgb::context().create_swap_chain(cgb::context().main_window(), cgb::context().mTmpSurface);
	//	cgb::context().mSurfSwap.emplace_back(std::make_unique<cgb::swap_chain_data>(std::move(swapChain)));

	//	mSwapChainData = cgb::context().get_surf_swap_tuple_for_window(cgb::context().main_window());
	//	assert(mSwapChainData);

	//	mPipeline = cgb::context().create_graphics_pipeline_for_window(
	//		shaderInfos,
	//		cgb::context().main_window(),
	//		cgb::image_format(mDepthImage->mInfo.format),
	//		Vertex::binding_description(),
	//		vertexAttribDesc.size(),
	//		vertexAttribDesc.data(),
	//		{ mDescriptorSetLayout.mDescriptorSetLayout });
	//	mFrameBuffers = cgb::context().create_framebuffers(mPipeline.mRenderPass, cgb::context().main_window(), mDepthImageView);
	//	mCmdBfrs = cgb::context().create_command_buffers_for_graphics(mFrameBuffers.size());
	//}

	void update() override
	{
		if (cgb::input().key_pressed(cgb::key_code::h)) {
			LOG_INFO_EM("Hello cg_base!");
		}
		if (cgb::input().key_pressed(cgb::key_code::c)) {
			cgb::context().main_window()->set_cursor_pos({ 666.0, 100 });
		}
		if (cgb::input().key_pressed(cgb::key_code::escape)) {
			cgb::current_composition().stop();
		}
	}

private:
	cgb::swap_chain_data* mSwapChainData;
	cgb::pipeline mPipeline;

};

int main()
{
	try {

		auto selectImageFormat = cgb::context_specific_function<cgb::image_format()>{}
			.SET_VULKAN_FUNCTION([]() { return cgb::image_format(vk::Format::eR8G8B8Unorm); })
			.SET_OPENGL46_FUNCTION([]() { return cgb::image_format{ GL_RGB };  });


		cgb::settings::gApplicationName = "Hello World";

		// Create a window which we're going to use to render to
		auto mainWnd = cgb::context().create_window("Hello World!");
		mainWnd->set_resolution({ 1600, 900 });
		mainWnd->set_presentaton_mode(cgb::presentation_mode::vsync);
		mainWnd->open(); 

		// Create a "behavior" which contains functionality of our program
		auto helloBehavior = hello_behavior();

		// Create a composition of all things that define the essence of 
		// our program, which there are:
		//  - a timer
		//  - an executor
		//  - a window
		//  - a behavior
		auto hello = cgb::composition<cgb::varying_update_timer, cgb::sequential_executor>({
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


