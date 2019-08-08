
#include "TexLoadingHelper.h"
#include "string_utils.h"


namespace cgb
{
	TexLoadingHelper::TexLoadingHelper(std::string model_path)
		: m_base_dir(extract_base_path(model_path)),
		m_general_tex_params(TexParams_AnisotropicFilteringMax)
	{

	}

	void TexLoadingHelper::AddTexParams(TexParams to_add)
	{
		m_general_tex_params |= to_add;
	}

	void TexLoadingHelper::RemoveTexParams(TexParams to_remove)
	{
		m_general_tex_params &= ~to_remove;
	}

	unsigned int TexLoadingHelper::general_tex_params() const
	{
		return m_general_tex_params;
	}

	std::shared_ptr<vulkan_texture> TexLoadingHelper::GetOrLoadTex(std::string sub_path, unsigned int texture_parameters)
	{
		std::string complete_path;
		assert(sub_path.length() > 1);
		const auto first_char = sub_path[0];
		if (first_char == SEP_TO_USE || first_char == SEP_NOT_TO_USE)
			complete_path = clean_up_path(m_base_dir + sub_path.substr(1));
		else
			complete_path = clean_up_path(m_base_dir + sub_path);

		// Is the texture already in our cache?
		auto search = m_tex_cache.find(complete_path);
		if (search != m_tex_cache.end())
		{
			return search->second;
		}
		else
		{
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load(complete_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

			if (!pixels) {
				LOG_ERROR("Failed to load LDR image from path [%s]", complete_path.c_str());
			}

			// TODO validate channels set correct channels on texture, currently default of texture is used!
			//m_data_type = GL_UNSIGNED_BYTE;
			//if (3 == m_color_channels)
			//{
			//	m_image_format = GL_RGB;
			//}
			//else if (4 == m_color_channels)
			//{
			//	m_image_format = GL_RGBA;
			//}
			//else if (1 == m_color_channels)
			//{
			//	m_image_format = GL_RED;
			//}
			//else if (2 == m_color_channels)
			//{
			//	m_image_format = GL_RG;
			//}
			//else
			//{
			//	log_warning("Strange color channel count of [%d]", m_color_channels);
			//}

			// TODO use texture_parameters

			auto textureImage = std::make_shared<cgb::vulkan_image>(pixels, texWidth, texHeight, texChannels);
			auto tex = std::make_shared<cgb::vulkan_texture>(textureImage);
			//tex->FromFile(complete_path).UploadSRGBIfPossible().SetTextureParameters(texture_parameters).GenerateMipMaps();

			stbi_image_free(pixels);
			m_tex_cache[complete_path] = tex;
			return tex;
		}
	}
}
