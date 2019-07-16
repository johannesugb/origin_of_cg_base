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

	static bool operator ==(const shader_info& first, const shader_info& second)
	{
		return first.mPath == second.mPath && first.mEntryPoint == second.mEntryPoint;
	}

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

	shader_info vertex_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::vertex); }
	shader_info tessellation_control_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::tessellation_control); }
	shader_info tessellation_evaluation_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::tessellation_evaluation); }
	shader_info geometry_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::geometry); }
	shader_info fragment_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::fragment); }
	shader_info compute_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::compute); }
	shader_info ray_generation_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::ray_generation); }
	shader_info any_hit_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::any_hit); }
	shader_info closest_hit_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::closest_hit); }
	shader_info miss_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::miss); }
	shader_info intersection_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::intersection); }
	shader_info callable_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::callable); }
	shader_info task_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::task); }
	shader_info mesh_shader(std::string pPath, std::string pEntryPoint = "main", bool pDontMonitorFile = false) { return shader_info::create(std::move(pPath), std::move(pEntryPoint), pDontMonitorFile, cgb::shader_type::mesh); }

}
