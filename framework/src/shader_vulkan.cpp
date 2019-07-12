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
		result.mTracker.setTrackee(result);
		return result;
	}



}
