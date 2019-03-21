#pragma once
#include "vulkan_texture.h"
#include "TexInfo.h"

namespace cgb
{
	
	class TexLoadingHelper
	{
	public:
		TexLoadingHelper(std::string model_path);
		std::shared_ptr<vulkan_texture> GetOrLoadTex(std::string sub_path, unsigned int texture_parameters);
		void AddTexParams(TexParams to_add);
		void RemoveTexParams(TexParams to_remove);
		unsigned int general_tex_params() const;

	private:
		unsigned int m_general_tex_params;
		std::string m_base_dir;
		std::unordered_map<std::string, std::shared_ptr<vulkan_texture>> m_tex_cache;
	};

}
