#pragma once
#ifdef USE_OPENGL46_CONTEXT
#include "context_generic_glfw.h"
#include "context_generic_opengl.h"

namespace cgb
{
	/**	\brief Context for OpenGL 4.6 core profile
	 *	
	 *	This context uses OpenGL 4.6 functionality and a core profile.
	 *	Some data and functionality is shared from the generic_opengl 
	 *  context, environment-related matters like window creation is
	 *	performed via GLFW, most of which is implemented in generic_glfw
	 *  context.
	 */
	class opengl46 : public generic_opengl, public generic_glfw
	{
	public:
		opengl46();

		window create_window();
	};
}

#endif // USE_OPENGL46_CONTEXT
