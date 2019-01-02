// hello_world.cpp : Defines the entry point for the console application.
//
#include "cg_base.h"
using namespace std;

class hello_behavior : public cgb::cg_element
{
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

#ifdef USE_VULKAN_CONTEXT
		auto swapChainData = cgb::context().get_surf_swap_tuple_for_window(mainWnd);
		assert(swapChainData);

		auto vert = cgb::shader_handle::create_from_binary_code(cgb::load_binary_file("shader/shader.vert.spv"));
		auto frag = cgb::shader_handle::create_from_binary_code(cgb::load_binary_file("shader/shader.frag.spv"));
		// PROBLEME:
		// - shader_handle* sollte kein Pointer sein!
		std::vector<std::tuple<cgb::shader_type, cgb::shader_handle*>> shaderInfos;
		shaderInfos.push_back(std::make_tuple(cgb::shader_type::vertex, &vert));
		shaderInfos.push_back(std::make_tuple(cgb::shader_type::fragment, &frag));
		auto pipeline = cgb::context().create_graphics_pipeline_for_window(shaderInfos, mainWnd);
		auto framebfrs = cgb::context().create_framebuffers(pipeline.mRenderPass, mainWnd);
		auto cmdpool = cgb::context().create_command_pool();
		auto cmdbfrs = cgb::context().create_command_buffers(static_cast<uint32_t>(framebfrs.size()), cmdpool);
		for (auto i = 0; i < cmdbfrs.size(); ++i) { // TODO: WTF, this must be abstracted somehow!
			auto& cmdbfr = cmdbfrs[i];
			cmdbfr.begin_recording();
			cmdbfr.begin_render_pass(pipeline.mRenderPass, framebfrs[i].mFramebuffer, { 0, 0 }, swapChainData->mSwapChainExtent);
			cgb::context().draw_triangle(pipeline, cmdbfr);
			cmdbfr.end_render_pass();
			cmdbfr.end_recording();
		}
#endif

		// Create a "behavior" which contains functionality of our program
		auto helloBehavior = hello_behavior();

		// Create a composition of all things that define the essence of 
		// our program, which there are:
		//  - a timer
		//  - an executor
		//  - a window
		//  - a behavior
		auto hello = cgb::composition<cgb::fixed_update_timer, cgb::sequential_executor>({
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


