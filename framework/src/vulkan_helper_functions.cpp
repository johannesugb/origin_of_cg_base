namespace cgb
{
	vk::IndexType to_vk_index_type(size_t pSize)
	{
		if (pSize == sizeof(uint16_t)) {
			return vk::IndexType::eUint16;
		}
		if (pSize == sizeof(uint32_t)) {
			return vk::IndexType::eUint32;
		}
		LOG_ERROR(fmt::format("The given size[{}] does not correspond to a valid vk::IndexType", pSize));
		return vk::IndexType::eNoneNV;
	}

	vk::ImageViewType to_image_view_type(const vk::ImageCreateInfo& info)
	{
		switch (info.imageType)
		{
		case vk::ImageType::e1D:
			if (info.arrayLayers > 1) {
				return vk::ImageViewType::e1DArray;
			}
			else {
				return vk::ImageViewType::e1D;
			}
		case vk::ImageType::e2D:
			if (info.arrayLayers > 1) {
				return vk::ImageViewType::e2DArray;
			}
			else {
				return vk::ImageViewType::e2D;
			}
		case vk::ImageType::e3D:
			return vk::ImageViewType::e3D;
		}
		throw new std::runtime_error("It might be that the implementation of to_image_view_type(const vk::ImageCreateInfo& info) is incomplete. Please complete it!");
	}

	vk::Bool32 to_vk_bool(bool value)
	{
		return value ? VK_TRUE : VK_FALSE;
	}

	vk::ShaderStageFlagBits to_vk_shader_stage(shader_type pType)
	{
		switch (pType) {
		case cgb::shader_type::vertex:
			return vk::ShaderStageFlagBits::eVertex;
		case cgb::shader_type::tessellation_control:
			return vk::ShaderStageFlagBits::eTessellationControl;
		case cgb::shader_type::tessellation_evaluation:
			return vk::ShaderStageFlagBits::eTessellationEvaluation;
		case cgb::shader_type::geometry:
			return vk::ShaderStageFlagBits::eGeometry;
		case cgb::shader_type::fragment:
			return vk::ShaderStageFlagBits::eFragment;
		case cgb::shader_type::compute:
			return vk::ShaderStageFlagBits::eCompute;
		case cgb::shader_type::ray_generation:
			return vk::ShaderStageFlagBits::eRaygenNV;
		case cgb::shader_type::any_hit:
			return vk::ShaderStageFlagBits::eAnyHitNV;
		case cgb::shader_type::closest_hit:
			return vk::ShaderStageFlagBits::eClosestHitNV;
		case cgb::shader_type::miss:
			return vk::ShaderStageFlagBits::eMissNV;
		case cgb::shader_type::intersection:
			return vk::ShaderStageFlagBits::eIntersectionNV;
		case cgb::shader_type::callable:
			return vk::ShaderStageFlagBits::eCallableNV;
		case cgb::shader_type::task:
			return vk::ShaderStageFlagBits::eTaskNV;
		case cgb::shader_type::mesh:
			return vk::ShaderStageFlagBits::eMeshNV;
		default:
			throw std::runtime_error("Invalid shader_type");
		}
	}

	vk::SampleCountFlagBits to_vk_sample_count(int pSampleCount)
	{
		switch (pSampleCount) {
		case 1:
			return vk::SampleCountFlagBits::e1;
		case 2:
			return vk::SampleCountFlagBits::e2;
		case 4:
			return vk::SampleCountFlagBits::e4;
		case 8:
			return vk::SampleCountFlagBits::e8;
		case 16:
			return vk::SampleCountFlagBits::e16;
		case 32:
			return vk::SampleCountFlagBits::e32;
		case 64:
			return vk::SampleCountFlagBits::e64;
		default:
			throw std::invalid_argument("Invalid number of samples");
		}
	}

	vk::VertexInputRate to_vk_vertex_input_rate(input_binding_general_data::kind _Value)
	{
		switch (_Value) {
		case input_binding_general_data::kind::instance:
			return vk::VertexInputRate::eInstance;
		case input_binding_general_data::kind::vertex:
			return vk::VertexInputRate::eVertex;
		default:
			throw std::invalid_argument("Invalid vertex input rate");
		}
	}
}
