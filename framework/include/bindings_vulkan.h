#pragma once
#include <type_traits>

namespace cgb
{
	template <typename T>
	vk::DescriptorType descriptor_type_of(const T& pResource);
	template <typename T>
	vk::DescriptorType descriptor_type_of(const T* pResource);

	template<>
	vk::DescriptorType descriptor_type_of<uniform_buffer_t>(const uniform_buffer_t& pResource) { return vk::DescriptorType::eUniformBuffer; }
	template<>
	vk::DescriptorType descriptor_type_of<uniform_buffer_t>(const uniform_buffer_t* pResource) { return vk::DescriptorType::eUniformBuffer; }

	template<>
	vk::DescriptorType descriptor_type_of<uniform_texel_buffer_t>(const uniform_texel_buffer_t& pResource) { return vk::DescriptorType::eUniformTexelBuffer; }
	template<>
	vk::DescriptorType descriptor_type_of<uniform_texel_buffer_t>(const uniform_texel_buffer_t* pResource) { return vk::DescriptorType::eUniformTexelBuffer; }

	template<>
	vk::DescriptorType descriptor_type_of<storage_buffer_t>(const storage_buffer_t& pResource) { return vk::DescriptorType::eStorageBuffer; }
	template<>
	vk::DescriptorType descriptor_type_of<storage_buffer_t>(const storage_buffer_t* pResource) { return vk::DescriptorType::eStorageBuffer; }

	template<>
	vk::DescriptorType descriptor_type_of<storage_texel_buffer_t>(const storage_texel_buffer_t& pResource) { return vk::DescriptorType::eStorageTexelBuffer; }
	template<>
	vk::DescriptorType descriptor_type_of<storage_texel_buffer_t>(const storage_texel_buffer_t* pResource) { return vk::DescriptorType::eStorageTexelBuffer; }

	template<>
	vk::DescriptorType descriptor_type_of<image_view_t>(const image_view_t& pResource) { return vk::DescriptorType::eStorageImage; }
	template<>
	vk::DescriptorType descriptor_type_of<image_view_t>(const image_view_t* pResource) { return vk::DescriptorType::eStorageImage; }

	template<>
	vk::DescriptorType descriptor_type_of<image_sampler_t>(const image_sampler_t& pResource) { return vk::DescriptorType::eCombinedImageSampler; }
	template<>
	vk::DescriptorType descriptor_type_of<image_sampler_t>(const image_sampler_t* pResource) { return vk::DescriptorType::eCombinedImageSampler; }

	template<>
	vk::DescriptorType descriptor_type_of<acceleration_structure>(const acceleration_structure& pResource) { return vk::DescriptorType::eAccelerationStructureNV; }
	template<>
	vk::DescriptorType descriptor_type_of<acceleration_structure>(const acceleration_structure* pResource) { return vk::DescriptorType::eAccelerationStructureNV; }

	template <typename T>
	binding_data binding(uint32_t pSet, uint32_t pBinding, const T& pResource, shader_type pShaderStages = shader_type::all)
	{
		binding_data data{
			pSet,
			vk::DescriptorSetLayoutBinding{}
				.setBinding(pBinding)
				.setDescriptorCount(num_elements(pResource)) // TODO: In which cases might this not be 1?
				.setDescriptorType(descriptor_type_of(first_or_only_element(pResource)))
				.setPImmutableSamplers(nullptr), // The pImmutableSamplers field is only relevant for image sampling related descriptors [3]
			pShaderStages
		};
		return data;
	}

	template <typename T>
	binding_data binding(uint32_t pBinding, const T& pResource, shader_type pShaderStages = shader_type::all)
	{
		return binding(0u, pBinding, pResource, pShaderStages);
	}

	template <typename T, typename TU, typename TS>
	binding_data binding(uint32_t pSet, uint32_t pBinding, const std::variant<T, TU, TS>& pResource, shader_type pShaderStages = shader_type::all)
	{
		return binding(pSet, pBinding, cgb::get(pResource), pShaderStages);
	}

	template <typename T, typename TU, typename TS>
	binding_data binding(uint32_t pBinding, const std::variant<T, TU, TS>& pResource, shader_type pShaderStages = shader_type::all)
	{
		return binding(0u, pBinding, cgb::get(pResource), pShaderStages);
	}
}
