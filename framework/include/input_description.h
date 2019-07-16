#pragma once

namespace cgb
{
	/** One specific input location description to a graphics pipeline,
	 *	used to gather data for the creation of a `input_description` instance.
	 */
	struct input_binding_location_data
	{
		enum struct kind { vertex, instance };

		uint32_t mBinding;
		size_t mStride;
		kind mKind;
		buffer_element_member_meta mLocation;
	};

	/**	Describes the input to a graphics pipeline
	 */
	class input_description
	{
		// The buffers at the binding locations 0, 1, ...
		// Each one of those buffer-metas contains the separate locations as childs.
		using input_buffer_t = std::variant<std::monostate, vertex_buffer_meta, instance_buffer_meta>;

	public:
		/** Create a complete input description record based on multiple `input_binding_location_data` records. */
		static input_description create(std::initializer_list<input_binding_location_data> pBindings);

	private:
		// Contains all the data, ordered by the binding ids
		// (and internally ordered by locations)
		std::map<uint32_t, input_buffer_t> mInputBuffers;
	};

	/** Describe an input location for a pipeline's vertex input.
	 *	Also, assign the input location to a specific binding point (first parameter `pBinding`).
	 *	The binding point represents a specific buffer which provides the data for the location specified.
	 */
	input_binding_location_data vertex_input_binding(uint32_t pBinding, uint32_t pLocation, size_t pOffset, buffer_member_format pFormat, size_t pStride)
	{
		return input_binding_location_data{ pBinding, pStride, input_binding_location_data::kind::vertex, { pLocation, pOffset, pFormat } };
	}

#if defined _MSC_VER && !defined _CRT_USE_BUILTIN_OFFSETOF
#ifdef __cplusplus
	/** Describe an input location for a pipeline's vertex input.
	 *	Also, assign the input location to a specific binding point (first parameter `pBinding`).
	 *	The binding point represents a specific buffer which provides the data for the location specified.
	 *	
	 *	Usage example:
	 *	```
	 *  {
	 *		vertex_input_binding(0, 0, &Vertex::pos);
	 *		vertex_input_binding(0, 1, &Vertex::color);
	 *		vertex_input_binding(0, 2, &Vertex::texCoord);
	 *  }
	 *	```
	 */
	template <class T, class M> 
	input_binding_location_data& vertex_input_binding(uint32_t pBinding, uint32_t pLocation, M T::* pMember)
	{
		return vertex_input_binding(
			pBinding, 
			pLocation, 
			((::size_t)&reinterpret_cast<char const volatile&>((((T*)0)->*pMember))),
			format_for<M>(),
			sizeof(T));
	}
#endif
#endif


	/** Describe an input location for a pipeline's instance input.
	*	Also, assign the input location to a specific binding point (first parameter `pBinding`).
	*	The binding point represents a specific buffer which provides the data for the location specified.
	*/
	input_binding_location_data instance_input_binding(uint32_t pBinding, uint32_t pLocation, size_t pOffset, buffer_member_format pFormat, size_t pStride)
	{
		return input_binding_location_data{ pBinding, pStride, input_binding_location_data::kind::instance, { pLocation, pOffset, pFormat } };
	}

#if defined _MSC_VER && !defined _CRT_USE_BUILTIN_OFFSETOF
#ifdef __cplusplus
	/** Describe an input location for a pipeline's instance input.
	*	Also, assign the input location to a specific binding point (first parameter `pBinding`).
	*	The binding point represents a specific buffer which provides the data for the location specified.
	*	
	*	Usage example:
	*	```
	*  {
	*		instance_input_binding(0, 0, &SomeInstanceData::color);
	*  }
	*	```
	*/
	template <class T, class M> 
	input_binding_location_data& instance_input_binding(uint32_t pBinding, uint32_t pLocation, M T::* pMember)
	{
		return instance_input_binding(
			pBinding, 
			pLocation, 
			((::size_t)&reinterpret_cast<char const volatile&>((((T*)0)->*pMember))),
			format_for<M>(),
			sizeof(T));
	}
#endif
#endif
}
