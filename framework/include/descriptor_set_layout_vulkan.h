#pragma once
#include <type_traits>

namespace cgb
{
#pragma region detect .size() member via SFINAE (if we'll ever need this)
	// SFINAE test
	template <typename T>
	class has_size_member
	{
	private:
		typedef char YesType[1];
		typedef char NoType[2];

		template <typename C> static YesType& test( decltype(&C::size) ) ;
		template <typename C> static NoType& test(...);

	public:
		enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
	};

	// SFINAE test
	template <typename T>
	class has_no_size_member
	{
	private:
		typedef char YesType[1];
		typedef char NoType[2];

		template <typename C> static YesType& test( decltype(&C::size) ) ;
		template <typename C> static NoType& test(...);

	public:
		enum { value = sizeof(test<T>(0)) != sizeof(YesType) };
	};

	template<typename T> 
	typename std::enable_if<has_size_member<T>::value, uint32_t>::type num_elements(const T& t) {
		return static_cast<uint32_t>(t.size());
	}

	template<typename T> 
	typename std::enable_if<has_no_size_member<T>::value, uint32_t>::type num_elements(const T& t) {
		return 1u;
	}
#pragma endregion

	struct binding_data
	{
		uint32_t mSetId;
		vk::DescriptorSetLayoutBinding mLayoutBinding;
	};

	bool operator ==(const binding_data& first, const binding_data& second)
	{
		return first.mSetId == second.mSetId
			&& first.mLayoutBinding == second.mLayoutBinding;
	}

	template <typename T>
	vk::DescriptorType descriptor_type_of(T&& pResource);
	template<>
	vk::DescriptorType descriptor_type_of<uniform_buffer_t>(uniform_buffer_t&& pResource) { return vk::DescriptorType::eUniformBuffer; }
	template<>
	vk::DescriptorType descriptor_type_of<uniform_texel_buffer_t>(uniform_texel_buffer_t&& pResource) { return vk::DescriptorType::eUniformTexelBuffer; }
	template<>
	vk::DescriptorType descriptor_type_of<storage_buffer_t>(storage_buffer_t&& pResource) { return vk::DescriptorType::eStorageBuffer; }
	template<>
	vk::DescriptorType descriptor_type_of<storage_texel_buffer_t>(storage_texel_buffer_t&& pResource) { return vk::DescriptorType::eStorageTexelBuffer; }

	template <typename T>
	binding_data binding(uint32_t pSet, uint32_t pBinding, const T& pResource, shader_type pShaderStages)
	{
		binding_data data{
			pSet,
			vk::DescriptorSetLayoutBinding{}
				.setBinding(pBinding)
				.setDescriptorCount(num_elements(pResource)) // TODO: In which cases might this not be 1?
				.setDescriptorType(descriptor_type_of(pResource))
				.setPImmutableSamplers(nullptr); // The pImmutableSamplers field is only relevant for image sampling related descriptors [3]
		};
		return data;
	}

	template <typename T>
	binding_data binding(uint32_t pBinding, T&& pResource, shader_type pShaderStages)
	{
		return binding(0u, pBinding, std::forward(pR))
	}

	vk::UniqueDescriptorSetLayout layout_for(std::initializer_list<binding_data> pBindings)
	{
		std::vector<vk::DescriptorSetLayoutBinding> asdf;
		asdf.push_back(all...);
		auto createInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(static_cast<uint32_t>(asdf.size()))
			.setPBindings(asdf.data());
		return cgb::context().logical_device().createDescriptorSetLayoutUnique(createInfo);
	}

}
