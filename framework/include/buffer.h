#pragma once

namespace cgb
{
	/** This struct contains information for a generic buffer.
 	 */
	struct generic_buffer_data
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/** This struct contains information for a uniform buffer.
	*/
	struct uniform_buffer_data
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/** This struct contains information for a uniform texel buffer.
	*/
	struct uniform_texel_buffer_data
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/** This struct contains information for a storage buffer.
	*/
	struct storage_buffer_data
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/** This struct contains information for a storage texel buffer.
	*/
	struct storage_texel_buffer_data
	{
		size_t total_size() const { return mSize; }
		size_t mSize;
	};

	/**	This struct contains information for a buffer which is intended to be used as 
	 *	vertex buffer, i.e. vertex attributes provided to a shader.
	 */
	struct vertex_buffer_data
	{
		size_t sizeof_one_element() const { return mSizeOfOneElement; }
		size_t num_elements() const { return mNumElements; }
		size_t total_size() const { return sizeof_one_element() * num_elements(); }

		size_t mSizeOfOneElement;
		size_t mNumElements;
	};

	/**	This struct contains information for a buffer which is intended to be used as 
	 *	index buffer.
	 */
	struct index_buffer_data
	{
		size_t sizeof_one_element() const { return mSizeOfOneElement; }
		size_t num_elements() const { return mIndexCount; }
		size_t total_size() const { return sizeof_one_element() * num_elements(); }

		// Size of the data type which is used to represent one index element, 
		// this is typically uint16 or uint32
		size_t mSizeOfOneElement;

		// The number of indices in the buffer
		uint32_t mIndexCount;
	};

	// Forward declare templated cgb::buffer and set up some type defs.
	// The definition of the buffer will always be inside the context-specific implementation files.
	//template <typename Cfg>
	//struct buffer_t;

	using generic_buffer		= std::variant<buffer_t<generic_buffer_data>,		std::unique_ptr<buffer_t<generic_buffer_data>>,			std::shared_ptr<buffer_t<generic_buffer_data>>>;
	using uniform_buffer		= std::variant<buffer_t<uniform_buffer_data>,		std::unique_ptr<buffer_t<uniform_buffer_data>>,			std::shared_ptr<buffer_t<uniform_buffer_data>>>;
	using uniform_texel_buffer	= std::variant<buffer_t<uniform_texel_buffer_data>,	std::unique_ptr<buffer_t<uniform_texel_buffer_data>>,	std::shared_ptr<buffer_t<uniform_texel_buffer_data>>>;
	using storage_buffer		= std::variant<buffer_t<storage_buffer_data>,		std::unique_ptr<buffer_t<storage_buffer_data>>,			std::shared_ptr<buffer_t<storage_buffer_data>>>;
	using storage_texel_buffer	= std::variant<buffer_t<storage_texel_buffer_data>,	std::unique_ptr<buffer_t<storage_texel_buffer_data>>,	std::shared_ptr<buffer_t<storage_texel_buffer_data>>>;
	using vertex_buffer			= std::variant<buffer_t<vertex_buffer_data>,		std::unique_ptr<buffer_t<vertex_buffer_data>>,			std::shared_ptr<buffer_t<vertex_buffer_data>>>;
	using index_buffer			= std::variant<buffer_t<index_buffer_data>,			std::unique_ptr<buffer_t<index_buffer_data>>,			std::shared_ptr<buffer_t<index_buffer_data>>>;

	using buffer = std::variant<
		generic_buffer,
		uniform_buffer,
		uniform_texel_buffer,
		storage_buffer,
		storage_texel_buffer,
		vertex_buffer,
		index_buffer
	>;

	/** Gets a reference to the data stored in a variant, regardless of how it is stored/referenced there,
	 *	be it stored directly or referenced via a smart pointer.
	 */
	template <typename T, typename V>
	T& get(V v)	{
		if (std::holds_alternative<T>(v)) {
			return std::get<T>(v);
		}
		if (std::holds_alternative<std::unique_ptr<T>>(v)) {
			return *std::get<std::unique_ptr<T>>(v);
		}
		if (std::holds_alternative<std::shared_ptr<T>>(v)) {
			return *std::get<std::shared_ptr<T>>(v);
		}
		throw std::bad_variant_access();
	}

	using buffers = std::variant<
		std::vector<generic_buffer>,
		std::vector<uniform_buffer>,
		std::vector<uniform_texel_buffer>,
		std::vector<storage_buffer>,
		std::vector<storage_texel_buffer>,
		std::vector<vertex_buffer>,
		std::vector<index_buffer>
	>;

	/** Gets a reference to the data of a specific element at the given index, stored in 
	 *	a collection of variants, regardless of how it is stored/referenced there, be 
	 *	it stored directly or referenced via a smart pointer.
	 */
	template <typename T, typename V>
	T& get_at(V v, size_t index)	{
		return get<T>(v[i]);
	}
}
