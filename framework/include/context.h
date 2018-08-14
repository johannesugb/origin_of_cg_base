#pragma once
#include <vector>

#ifdef PLEASE_USE_OPENGL46
#include "context_opengl46.h"
#elif PLEASE_USE_OPENGLES32
#include "context_opengles32.h"
#elif PLEASE_USE_VULKAN
#include "context_vulkan.h"
#endif

namespace please
{

#ifdef PLEASE_USE_OPENGL46
	inline opengl46& context()
	{
		static opengl46 instance;
		return instance;
	}
#elif PLEASE_USE_OPENGLES32
	inline opengles32& context()
	{
		static opengles32 instance;
		return instance;
	}
#elif PLEASE_USE_VULKAN
	
#endif
}
