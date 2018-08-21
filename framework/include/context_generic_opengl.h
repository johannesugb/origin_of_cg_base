#pragma once
#if defined(USE_OPENGL46_CONTEXT) || defined(USE_OPENGLES32_CONTEXT)

namespace cgb
{
	// texture handle type for OpenGL contexts
	struct texture_handle
	{
		int m_fuckyou;
	};

	class generic_opengl
	{
	public:
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

#endif // defined(USE_OPENGL46_CONTEXT) || defined(USE_OPENGLES32_CONTEXT)
