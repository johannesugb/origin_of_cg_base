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
	try
	{
		cgb::settings::gApplicationName = "Hello World";
		cgb::settings::gApplicationVersion = cgb::make_version(1, 0, 0);
		cgb::settings::gRequiredDeviceExtensions.push_back("VK_NV_ray_tracing");

		// Create a window which we're going to use to render to
		cgb::window_params windowParams = {
			std::nullopt,
			std::nullopt,
			"Hello cg_base World!"
		};
		auto mainWnd = cgb::context().create_window(windowParams);

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


