#pragma once

namespace please
{
	class generic_opengl
	{
	public:
		// texture handle type for OpenGL contexts
		struct texture_handle
		{
			int m_fuckyou;
		};

		texture_handle create_texture()
		{
			return texture_handle();
		}

		void destroy_texture(texture_handle p_handle)
		{

		}
	};
}
