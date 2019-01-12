// context_completeness.cpp : Test contexts for completeness of functionality
//
#include "cg_base.h"

int main()
{
	using namespace cgb;

	// The idea is to call each and every function on the currently active context
	// so, by switching the configuration, one can detect potentially missing 
	// functionality on contexts.

	// == Window-related stuff ==
	auto windowHandle = window_handle{};
	auto wnd = window(windowHandle);
	auto monitorHandle = monitor_handle{};
	context().create_window(window_params{}, swap_chain_params{});
	context().close_window(wnd);
	context().get_time();
	
	// == Texture-related stuff ==
	auto textureHandle = context().create_texture();
	context().destroy_texture(textureHandle);

	// == Image formats ==
	auto imageType = image_format();
	assert(false == is_srgb_format(imageType));
#if defined(USE_OPENGL46_CONTEXT)
	auto imageTypeSrgb = image_format{ GL_SRGB8_ALPHA8 };
#elif defined(USE_VULKAN_CONTEXT)
	auto imageTypeSrgb = image_format( vk::Format::eR8G8B8A8Srgb );
#endif
	assert(true  == is_srgb_format(imageTypeSrgb));
	assert(true  == is_uint8_format(imageTypeSrgb));
	assert(false == is_int8_format(imageTypeSrgb));
	assert(false == is_uint16_format(imageTypeSrgb));
	assert(false == is_int16_format(imageTypeSrgb));
	assert(false == is_uint32_format(imageTypeSrgb));
	assert(false == is_int32_format(imageTypeSrgb));
	assert(false == is_float16_format(imageTypeSrgb));
	assert(false == is_float32_format(imageTypeSrgb));
	assert(false == is_float64_format(imageTypeSrgb));
	assert(false == is_rgb_format(imageTypeSrgb));
	assert(true  == is_rgba_format(imageTypeSrgb));
	assert(false == is_argb_format(imageTypeSrgb));
	assert(false == is_bgr_format(imageTypeSrgb));
	assert(false == is_bgra_format(imageTypeSrgb));
	assert(false == is_abgr_format(imageTypeSrgb));
}

