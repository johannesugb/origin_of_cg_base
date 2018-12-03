#pragma once
#ifdef USE_OPENGL46_CONTEXT
#include "context_generic_glfw.h"
#include "context_generic_opengl.h"

namespace cgb
{
	// texture handle type for OpenGL contexts
	struct texture_handle
	{
		int m_gl_specific_handle;
	};

	/**	\brief Context for OpenGL 4.6 core profile
	 *	
	 *	This context uses OpenGL 4.6 functionality and a core profile.
	 *	Some data and functionality is shared from the generic_opengl 
	 *  context, environment-related matters like window creation is
	 *	performed via GLFW, most of which is implemented in generic_glfw
	 *  context.
	 */
	class opengl46 : public generic_glfw
	{
	public:
		opengl46();

		window create_window();

		/**	Checks whether there is a GL-Error and logs it to the console
				 *	@return true if there was an error, false if not
				 */
		bool check_error(const char* file, int line);

		texture_handle create_texture()
		{
			return texture_handle();
		}

		void destroy_texture(texture_handle pHandle)
		{

		}
	};
}

#endif // USE_OPENGL46_CONTEXT
