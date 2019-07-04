#pragma once
#include <type_traits>

namespace cgb
{
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





	struct binding_data
	{
		uint32_t mSetId;
		vk::DescriptorSetLayoutBinding mLayoutBinding;
	};

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
				.setDescriptorCount(num_elements(pResource) // TODO: In which cases might this not be 1?
				.setDescriptorType(descriptor_type_of(pResource)
				.setPImmutableSamplers(nullptr); // The pImmutableSamplers field is only relevant for image sampling related descriptors [3]
		};
		return data;
	}

	template <typename T>
	binding_data binding(uint32_t pBinding, T&& pResource, shader_type pShaderStages)
	{
		return binding(0u, pBinding, std::forward(pR))
	}


	struct descriptor_set_layout
	{
		descriptor_set_layout() noexcept;
		descriptor_set_layout(const vk::DescriptorSetLayout& pDescriptorSetLayout);
		descriptor_set_layout(const descriptor_set_layout&) = delete;
		descriptor_set_layout(descriptor_set_layout&&) noexcept;
		descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;
		descriptor_set_layout& operator=(descriptor_set_layout&&) noexcept;
		~descriptor_set_layout();

		//static descriptor_set_layout create(const vk::DescriptorSetLayoutCreateInfo& pCreateInfo);
		static descriptor_set_layout create_for(std::initializer_list<binding_data> pBindings);

		vk::DescriptorSetLayout mDescriptorSetLayout;
	};

}
