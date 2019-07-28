#pragma once

namespace cgb
{
	template <typename V, typename F>
	bool has_flag(V value, F flag)
	{
		return (value & flag) == flag;
	}

	extern vk::IndexType to_vk_index_type(size_t pSize);

	extern vk::ImageViewType to_image_view_type(const vk::ImageCreateInfo& info);

	extern vk::Bool32 to_vk_bool(bool value);

	/** Converts a cgb::shader_type to the vulkan-specific vk::ShaderStageFlagBits type */
	extern vk::ShaderStageFlagBits to_vk_shader_stage(shader_type pType);

	extern vk::SampleCountFlagBits to_vk_sample_count(int pSampleCount);

	extern int to_cgb_sample_count(vk::SampleCountFlagBits pSampleCount);

	extern vk::VertexInputRate to_vk_vertex_input_rate(input_binding_general_data::kind _Value);
	
	extern vk::PrimitiveTopology to_vk_primitive_topology(primitive_topology _Value);

	extern vk::PolygonMode to_vk_polygon_mode(polygon_drawing_mode _Value);

	extern vk::CullModeFlags to_vk_cull_mode(culling_mode _Value);

	extern vk::FrontFace to_vk_front_face(winding_order _Value);

	extern vk::CompareOp to_vk_compare_op(compare_operation _Value);

	extern vk::ColorComponentFlags to_vk_color_components(color_channel _Value);

	extern vk::BlendFactor to_vk_blend_factor(blending_factor _Value);

	extern vk::BlendOp to_vk_blend_operation(color_blending_operation _Value);

	extern vk::LogicOp to_vk_logic_operation(blending_logic_operation _Value);
}
