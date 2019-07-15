#pragma once

namespace cgb
{
	struct shader_info
	{
		static shader_info create(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false, std::optional<cgb::shader_type> pShaderType = {});

		std::string mPath;
		cgb::shader_type mShaderType;
		std::string mEntryPoint;
		bool mDontMonitorFile;
	};


	/** @brief Shader source information and shader loading options
	 *
	 *	This information is important especially for shader hot reloading.
	 */
	enum struct shader_source_info : uint32_t
	{
		nothing			= 0x0000,
		/** Shader source is loaded from a file */
		from_file		= 0x0001,
		/** Shader source is loaded from memory (a string most likely) */
		from_memory		= 0x0002,
		/** Load the shader and append a new-line to the source */
		append_newline	= 0x0004,
	};

	inline shader_source_info operator| (shader_source_info a, shader_source_info b)
	{
		typedef std::underlying_type<shader_source_info>::type EnumType;
		return static_cast<shader_source_info>(static_cast<EnumType>(a) | static_cast<EnumType>(b));
	}

	inline shader_source_info operator& (shader_source_info a, shader_source_info b)
	{
		typedef std::underlying_type<shader_source_info>::type EnumType;
		return static_cast<shader_source_info>(static_cast<EnumType>(a) & static_cast<EnumType>(b));
	}

	inline shader_source_info& operator |= (shader_source_info& a, shader_source_info b)
	{
		return a = a | b;
	}

	inline shader_source_info& operator &= (shader_source_info& a, shader_source_info b)
	{
		return a = a & b;
	}
}
