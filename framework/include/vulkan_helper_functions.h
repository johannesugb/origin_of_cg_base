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

	extern vk::VertexInputRate to_vk_vertex_input_rate(input_binding_general_data::kind _Value);
}
