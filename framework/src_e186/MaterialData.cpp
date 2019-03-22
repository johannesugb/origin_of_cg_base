#include "MaterialData.h"
#include "MaterialUniformLocation.h"

namespace cgb
{
	std::shared_ptr<vulkan_resource_bundle_layout> MaterialData::create_resource_bundle_layout()
	{
		auto resourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		int binding = 0;
		resourceBundleLayout->add_binding(binding++, vk::DescriptorType::eUniformBuffer, cgb::ShaderStageFlagBits::eFragment);
		for (int i = 0; i < TEX_COUNT; i++) {
			resourceBundleLayout->add_binding(binding++, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		}
		resourceBundleLayout->bake();
		return resourceBundleLayout;
	}

	MaterialData::MaterialData() :
		m_name(""),
		m_hidden(false),
		m_diffuse_reflectivity(1.0f, 1.0f, 1.0f),
		m_specular_reflectivity(1.0f, 1.0f, 1.0f),
		m_ambient_reflectivity(1.0f, 1.0f, 1.0f),
		m_emissive_color(1.0f, 1.0f, 1.0f),
		m_transparent_color(1.0f, 1.0f, 1.0f),
		m_wireframe_mode(false),
		m_twosided(false),
		m_blend_mode(BlendMode::AlphaBlended),
		m_opacity(1.0f),
		m_shininess(0.0f),
		m_shininess_strength(0.0f),
		m_refraction_index(0.0f),
		m_reflectivity(0.0f),
		m_albedo(0.0f, 0.0f, 0.0f),
		m_metallic(0.0f),
		m_smoothness(0.0f),
		m_roughness(0.0f),
		m_tiling(1.0f, 1.0f),
		m_offset(0.0f, 0.0f)
	{ 
	}

	TexParams MaterialData::ai_mapping_mode_to_tex_params(aiTextureMapMode aimm)
	{
		switch (aimm)
		{
		case aiTextureMapMode_Wrap:
			return TexParams_Repeat;
		case aiTextureMapMode_Clamp:
			return TexParams_ClampToEdge;
		case aiTextureMapMode_Decal:
			return TexParams_ClampToEdge;
		case aiTextureMapMode_Mirror:
			return TexParams_MirroredRepeat;
		default:
			return TexParams_Repeat;
		}
	}

	MaterialData::MaterialData(aiMaterial* aimat, TexLoadingHelper& tlh) : MaterialData()
	{
		aiString strVal;
		aiColor3D color(0.0f, 0.0f, 0.0f);
		int intVal;
		aiBlendMode blendMode;
		float floatVal;
		aiTextureMapping texMapping;
		aiTextureMapMode texMapMode;
		
		if (AI_SUCCESS == aimat->Get(AI_MATKEY_NAME, strVal)) {
			m_name = strVal.data;
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
			m_diffuse_reflectivity = glm::vec3(color.r, color.g, color.b);
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_COLOR_SPECULAR, color)) {
			m_specular_reflectivity = glm::vec3(color.r, color.g, color.b);
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_COLOR_AMBIENT, color)) {
			m_ambient_reflectivity = glm::vec3(color.r, color.g, color.b);
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_COLOR_EMISSIVE, color)) {
			m_emissive_color = glm::vec3(color.r, color.g, color.b);
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_ENABLE_WIREFRAME, intVal)) {
			m_wireframe_mode = 0 != intVal;
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_TWOSIDED, intVal)) {
			m_twosided = 0 != intVal;
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_BLEND_FUNC, blendMode)) {
			m_blend_mode = blendMode == aiBlendMode_Additive
				? BlendMode::Additive
				: BlendMode::AlphaBlended;
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_OPACITY, floatVal)) {
			m_opacity = floatVal;
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_SHININESS, floatVal)) {
			m_shininess = floatVal;
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_SHININESS_STRENGTH, floatVal)) {
			m_shininess_strength = floatVal;
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_REFRACTI, floatVal)) {
			m_refraction_index = floatVal;
		}

		if (AI_SUCCESS == aimat->Get(AI_MATKEY_REFLECTIVITY, floatVal)) {
			m_reflectivity = floatVal;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_DIFFUSE, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode) | TexParams_GenerateMipMaps);
			m_diffuse_tex = tex;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_SPECULAR, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode) | TexParams_GenerateMipMaps);
			m_specular_tex = tex;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_AMBIENT, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode) | TexParams_GenerateMipMaps);
			m_ambient_tex = tex;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_EMISSIVE, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode) | TexParams_GenerateMipMaps);
			m_emissive_tex = tex;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_HEIGHT, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode) | TexParams_GenerateMipMaps);
			m_height_tex = tex;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_NORMALS, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode) | TexParams_GenerateMipMaps);
			m_normals_tex = tex;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_SHININESS, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode) | TexParams_GenerateMipMaps);
			m_shininess_tex = tex;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_OPACITY, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode) | TexParams_GenerateMipMaps);
			m_opacity_tex = tex;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_DISPLACEMENT, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode));
			m_displacement_tex = tex;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_REFLECTION, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode) | TexParams_GenerateMipMaps);
			m_reflection_tex = tex;
		}

		if (AI_SUCCESS == aimat->GetTexture(aiTextureType_LIGHTMAP, 0, &strVal, &texMapping, nullptr, nullptr, nullptr, &texMapMode)) {
			if (texMapping != aiTextureMapping_UV) {
				LOG_WARNING("Unsupported texture mapping mode[%u] for texture[%s]", static_cast<unsigned int>(texMapping), strVal.C_Str());
			}
			auto tex = tlh.GetOrLoadTex(strVal.data, tlh.general_tex_params() | ai_mapping_mode_to_tex_params(texMapMode) | TexParams_GenerateMipMaps);
			m_lightmap_tex = tex;
		}
	}


	std::shared_ptr<vulkan_resource_bundle> cgb::MaterialData::create_resource_bundle(std::shared_ptr<vulkan_resource_bundle_group> resourceBundleGroup)
	{
		auto resourceBundle = resourceBundleGroup->create_resource_bundle(MaterialData::get_resource_bundle_layout(), false);

		auto materialUniform = material_uniform();
		materialUniform.diffuse_reflectivity = m_diffuse_reflectivity;
		materialUniform.specular_reflectivity = m_specular_reflectivity;
		materialUniform.ambient_reflectivity = m_ambient_reflectivity;
		materialUniform.emissive_color = m_emissive_color;
		materialUniform.transparent_color = m_transparent_color;
		materialUniform.wireframe_mode = m_wireframe_mode;
		materialUniform.twosided = m_twosided;
		materialUniform.blend_mode = m_blend_mode;
		materialUniform.opacity = m_opacity;
		materialUniform.shininess = m_shininess;
		materialUniform.shininess_strength = m_shininess_strength;
		materialUniform.refraction_index = m_refraction_index;
		materialUniform.reflectivity = m_reflectivity;

		materialUniform.albedo = m_albedo;
		materialUniform.metallic = m_metallic;
		materialUniform.smoothness = m_smoothness;
		materialUniform.sheen = m_sheen;
		materialUniform.thickness = m_thickness;
		materialUniform.roughness = m_roughness;
		materialUniform.anisotropy = m_anisotropy;
		materialUniform.anisotropy_rotation = m_anisotropy_rotation;
		materialUniform.offset = m_offset;
		materialUniform.tiling = m_tiling;

		vk::DeviceSize bufferSize = sizeof(material_uniform);
		auto uniformBuffer = std::make_shared<cgb::vulkan_buffer>(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		uniformBuffer->update_buffer(&materialUniform, sizeof(materialUniform));


		resourceBundle->add_buffer_resource(0, uniformBuffer, bufferSize);
		if (m_diffuse_tex) { resourceBundle->add_image_resource(1, vk::ImageLayout::eShaderReadOnlyOptimal, m_diffuse_tex); }
		if (m_specular_tex) { resourceBundle->add_image_resource(2, vk::ImageLayout::eShaderReadOnlyOptimal, m_specular_tex); }
		if (m_ambient_tex) { resourceBundle->add_image_resource(3, vk::ImageLayout::eShaderReadOnlyOptimal, m_ambient_tex); }
		if (m_emissive_tex) { resourceBundle->add_image_resource(4, vk::ImageLayout::eShaderReadOnlyOptimal, m_emissive_tex); }
		if (m_height_tex) { resourceBundle->add_image_resource(5, vk::ImageLayout::eShaderReadOnlyOptimal, m_height_tex); }
		if (m_normals_tex) { resourceBundle->add_image_resource(6, vk::ImageLayout::eShaderReadOnlyOptimal, m_normals_tex); }
		if (m_shininess_tex) { resourceBundle->add_image_resource(7, vk::ImageLayout::eShaderReadOnlyOptimal, m_shininess_tex); }
		if (m_opacity_tex) { resourceBundle->add_image_resource(8, vk::ImageLayout::eShaderReadOnlyOptimal, m_opacity_tex); }
		if (m_displacement_tex) { resourceBundle->add_image_resource(9, vk::ImageLayout::eShaderReadOnlyOptimal, m_displacement_tex); }
		if (m_reflection_tex) { resourceBundle->add_image_resource(10, vk::ImageLayout::eShaderReadOnlyOptimal, m_reflection_tex); }
		if (m_lightmap_tex) { resourceBundle->add_image_resource(11, vk::ImageLayout::eShaderReadOnlyOptimal, m_lightmap_tex); }

		return resourceBundle;
	}

}
