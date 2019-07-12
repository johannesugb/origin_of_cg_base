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

		bool has_been_built();

		static shader prepare(const std::string& pPath);
		static shader create_from_binary_file(const std::string& pPath);
		static shader create_from_binary_code(const std::vector<char>& pCode);

	private:
		std::string mPath;
		vk::UniqueShaderModule mShaderModule;
		context_tracker<shader> mTracker;
	};

}
