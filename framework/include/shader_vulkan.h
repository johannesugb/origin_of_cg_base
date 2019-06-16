#pragma once
#include "shader_type.h"

namespace cgb
{
	/** Represents a shader program handle for the Vulkan context */
	class shader
	{
	public:
		shader() = default;
		shader(const shader&) = delete;
		shader(shader&&) = default;
		shader& operator=(const shader&) = delete;
		shader& operator=(shader&&) = default;
		~shader() = default;

		const auto& handle() const { return mShaderModule.get(); }
		const auto* handle_addr() const { return &mShaderModule.get(); }

		static shader create_from_binary_code(const std::vector<char>& code);

	private:
		vk::UniqueShaderModule mShaderModule;
	};

	/** Converts a cgb::shader_type to the vulkan-specific vk::ShaderStageFlagBits type */
	extern vk::ShaderStageFlagBits convert(shader_type p);

}
