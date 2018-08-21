#include "context_generic_opengl.h"
#if defined(USE_OPENGL46_CONTEXT) || defined(USE_OPENGLES32_CONTEXT)

namespace cgb
{
	bool generic_opengl::check_error(const char* file, int line)
	{
		bool hasError = false;
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			LOG_ERROR("glError int[%d] hex[0x%x] in file[%s], line[%d]", err, err, file, line);
			hasError = true;
		}
		return hasError;
	}
}

#endif
