#pragma once

#include "context_opengl46.h"
#include "context_opengles32.h"
#include "context_vulkan.h"

namespace cgb
{
#if defined(USE_OPENGL46_CONTEXT)
	inline opengl46& context()
	{
		static opengl46 instance;
		return instance;
	}
#elif defined(USE_OPENGLES32_CONTEXT)
	inline opengles32& context()
	{
		static opengles32 instance;
		return instance;
	}
#elif defined(USE_VULKAN_CONTEXT)
	inline vulkan& context()
	{
		static vulkan instance;
		return instance;
	}
#endif

/** Calls the context's \ref check_error method, passing the current file and line to it. */
#define CHECK_ERROR() context().check_error(__FILE__, __LINE__);
}
