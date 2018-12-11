#pragma once

#if defined(USE_OPENGL46_CONTEXT)
#include "context_opengl46.h"
#elif defined(USE_VULKAN_CONTEXT)
#include "context_vulkan.h"
#endif

namespace cgb
{
	namespace settings
	{
		/** Set this to your application's name */
		extern std::string gApplicationName;

		/** Set this to your application's version */
		extern uint32_t gApplicationVersion;

		/** Fill this vector with further required instance extensions, if required */
		extern std::vector<const char*> gRequiredInstanceExtensions;

		/** Modify this vector according to your needs. 
		 *  It will be initialized with a default validation layer name already.
		 */
		extern std::vector<const char*> gValidationLayersToBeActivated;

		/** Fill this vector with required device extensions, if required */
		extern std::vector<const char*> gRequiredDeviceExtensions;
	}

#if defined(USE_OPENGL46_CONTEXT)
	inline opengl46& context()
	{
		static opengl46 instance;
		return instance;
	}
#elif defined(USE_VULKAN_CONTEXT)
	inline vulkan& context()
	{
		static vulkan instance;
		return instance;
	}
#endif

/** Calls the context's @ref check_error method, passing the current file and line to it. */
#define CHECK_ERROR() context().check_error(__FILE__, __LINE__);
}
