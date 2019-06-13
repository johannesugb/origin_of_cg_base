#include "shader_vulkan.h"

namespace cgb
{
	shader shader::create_from_binary_code(const std::vector<char>& code)
	{
		auto createInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(code.size())
			.setPCode(reinterpret_cast<const uint32_t*>(code.data()));
		shader result;
		result.mShaderModule = context().logical_device().createShaderModuleUnique(createInfo);
		return result;
	}

	vk::ShaderStageFlagBits convert(shader_type p)
	{
		switch (p) {
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

}
