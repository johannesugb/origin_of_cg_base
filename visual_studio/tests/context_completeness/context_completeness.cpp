// context_completeness.cpp : Test contexts for completeness of functionality
//
#include "cg_base.h"

int main()
{
	using namespace cgb;

	// The idea is to call each and every function on the currently active context
	// so, by switching the configuration, one can detect potentially missing 
	// functionality on contexts.

	auto windowHandle = window_handle{};
	auto wnd = window(windowHandle);
	auto monitorHandle = monitor_handle{};
	context().create_window(window_params{}, swap_chain_params{});
	context().close_window(wnd);
	context().get_time();
	//context().check_error("", 0);
	auto textureHandle = context().create_texture();
	context().destroy_texture(textureHandle);
}

