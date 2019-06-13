#pragma once

namespace cgb
{

	/** Represents one specific type of shader */
	enum struct shader_type
	{
		/** Vertex Shader */
		vertex,
		/** Tessellation Control Shader */
		tessellation_control,
		/** Tessellation Evaluation Shader */
		tessellation_evaluation,
		/** Geometry Shader */
		geometry,
		/** Fragment Shader */
		fragment,
		/** Compute Shader */
		compute,
		/** Ray Generation Shader (Nvidia RTX) */
		ray_generation,
		/** Ray Tracing Any Hit Shader  (Nvidia RTX) */
		any_hit,
		/** Ray Tracing Closest Hit Shader (Nvidia RTX) */
		closest_hit,
		/** Ray Tracing Miss Shader (Nvidia RTX) */
		miss,
		/** Ray Tracing Intersection Shader (Nvidia RTX) */
		intersection,
		/** Ray Tracing Callable Shader (Nvidia RTX) */
		callable,
		/** Task Shader (Nvidia Turing)  */
		task,
		/** Mesh Shader (Nvidia Turing)  */
		mesh
	};
}
