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

	vk::PrimitiveTopology to_vk_primitive_topology(primitive_topology _Value)
	{
		switch (_Value) {
		case primitive_topology::points:
			return vk::PrimitiveTopology::ePointList;
		case primitive_topology::lines: 
			return vk::PrimitiveTopology::eLineList;
		case primitive_topology::line_strip:
			return vk::PrimitiveTopology::eLineStrip;
		case primitive_topology::triangles: 
			return vk::PrimitiveTopology::eTriangleList;
		case primitive_topology::triangle_strip:
			return vk::PrimitiveTopology::eTriangleStrip;
		case primitive_topology::triangle_fan: 
			return vk::PrimitiveTopology::eTriangleFan;
		case primitive_topology::lines_with_adjacency:
			return vk::PrimitiveTopology::eLineListWithAdjacency;
		case primitive_topology::line_strip_with_adjacency: 
			return vk::PrimitiveTopology::eLineStripWithAdjacency;
		case primitive_topology::triangles_with_adjacency: 
			return vk::PrimitiveTopology::eTriangleListWithAdjacency;
		case primitive_topology::triangle_strip_with_adjacency: 
			return vk::PrimitiveTopology::eTriangleStripWithAdjacency;
		case primitive_topology::patches: 
			return vk::PrimitiveTopology::ePatchList;
		default:
			throw std::invalid_argument("Invalid primitive topology");
		}
	}

	vk::PolygonMode to_vk_polygon_mode(polygon_drawing_mode _Value)
	{
		switch (_Value) {
		case polygon_drawing_mode::fill: 
			return vk::PolygonMode::eFill;
		case polygon_drawing_mode::line:
			return vk::PolygonMode::eLine;
		case polygon_drawing_mode::point:
			return vk::PolygonMode::ePoint;
		default:
			throw std::invalid_argument("Invalid polygon drawing mode.");
		}
	}

	vk::CullModeFlags to_vk_cull_mode(culling_mode _Value)
	{
		switch (_Value) {
		case culling_mode::disabled:
			return vk::CullModeFlagBits::eNone;
		case culling_mode::cull_front_faces:
			return vk::CullModeFlagBits::eFront;
		case culling_mode::cull_back_faces:
			return vk::CullModeFlagBits::eBack;
		case culling_mode::cull_front_and_back_faces:
			return vk::CullModeFlagBits::eFrontAndBack;
		default:
			throw std::invalid_argument("Invalid culling mode.");
		}
	}

	vk::FrontFace to_vk_front_face(winding_order _Value)
	{
		switch (_Value) {
		case winding_order::counter_clockwise:
			return vk::FrontFace::eCounterClockwise;
		case winding_order::clockwise:
			return vk::FrontFace::eClockwise;
		default:
			throw std::invalid_argument("Invalid front face winding order.");
		}
	}

	vk::CompareOp to_vk_compare_op(compare_operation _Value)
	{
		switch(_Value) {
		case compare_operation::never:
			return vk::CompareOp::eNever;
		case compare_operation::less: 
			return vk::CompareOp::eLess;
		case compare_operation::equal: 
			return vk::CompareOp::eEqual;
		case compare_operation::less_or_equal: 
			return vk::CompareOp::eLessOrEqual;
		case compare_operation::greater: 
			return vk::CompareOp::eGreater;
		case compare_operation::not_equal: 
			return vk::CompareOp::eNotEqual;
		case compare_operation::greater_or_equal: 
			return vk::CompareOp::eGreaterOrEqual;
		case compare_operation::always: 
			return vk::CompareOp::eAlways;
		default:
			throw std::invalid_argument("Invalid compare operation.");
		}
	}
}
