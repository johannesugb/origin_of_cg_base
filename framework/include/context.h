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

		/** Set to true to not use IMGUI, i.e. not initialize it, nothing. */
		extern bool gDisableImGui;
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

	/** A struct holding a context-specific function
	 *	Create such a struct and use it in conjunction with the macros SET_VULKAN_FUNCTION and 
	 *	SET_OPENGL46_FUNCTION to specify context-specific code.
	 *
	 *	Example: Create a context-specific function to select a format depending on the context used:
	 *
	 *	 auto selectImageFormat = cgb::context_specific_function<cgb::image_format()>{}
	 *								.SET_VULKAN_FUNCTION([]() { return cgb::image_format{ vk::Format::eR8G8B8Unorm }; })
	 *								.SET_OPENGL46_FUNCTION([]() { return cgb::image_format{ GL_RGB }; });
	 */
	template<typename T>
	struct context_specific_function
	{
		std::function<T> mFunction;

		/** Sets the function. However, this is not intended to be called directly!
		 *	Use the macros SET_VULKAN_FUNCTION and SET_OPENGL46_FUNCTION instead!
		 *	Use those macros exactly in the same way as you would call this function;
		 *	i.e. instead of .set_function(...) just write .SET_VULKAN_FUNCTION(...) for the Vulkan context.
		 */
		auto& set_function(std::function<T> func)
		{
			mFunction = std::move(func);
			return *this;
		}

		/** Function which is used by the macros SET_VULKAN_FUNCTION and SET_OPENGL46_FUNCTION.
		 *	This is not intended to be called directly, so just use the macros!
		 */
		auto& do_nothing()
		{
			return *this;
		}
	};

#if defined(USE_OPENGL46_CONTEXT)
#define EXECUTE_OPENGL46_ONLY(x) x
#define EXECUTE_VULKAN_ONLY(x) 
#define SET_OPENGL46_FUNCTION(x) set_function(x)
#define SET_VULKAN_FUNCTION(x) do_nothing()
#elif defined(USE_VULKAN_CONTEXT)
#define EXECUTE_OPENGL46_ONLY(x)
#define EXECUTE_VULKAN_ONLY(x) x
#define SET_OPENGL46_FUNCTION(x) do_nothing()
#define SET_VULKAN_FUNCTION(x) set_function(x)
#endif

}
