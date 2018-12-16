#pragma once

// INCLUDES:
#include <glad/glad.h>
#include "context_generic_glfw.h"
#include "context_opengl46_types.h"

namespace cgb
{
	/**	@brief Context for OpenGL 4.6 core profile
	 *	
	 *	This context uses OpenGL 4.6 functionality and a core profile.
	 *	Some data and functionality is shared from the generic_opengl 
	 *  context, environment-related matters like window creation is
	 *	performed via GLFW, most of which is implemented in @ref generic_glfw
	 *  context.
	 */
	class opengl46 : public generic_glfw
	{
	public:
		opengl46();

		window* create_window(const window_params&, const swap_chain_params&);

		/**	Checks whether there is a GL-Error and logs it to the console
				 *	@return true if there was an error, false if not
				 */
		bool check_error(const char* file, int line);

		texture_handle create_texture()
		{
			return texture_handle();
		}

		void destroy_texture(const texture_handle& pHandle)
		{

		}
	};
}
