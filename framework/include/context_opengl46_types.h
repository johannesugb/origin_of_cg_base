#pragma once
#ifdef USE_OPENGL46_CONTEXT

namespace cgb
{
	// texture handle type for OpenGL contexts
	struct texture_handle
	{
		int m_gl_specific_handle;
	};
}

#endif // USE_OPENGL46_CONTEXT
