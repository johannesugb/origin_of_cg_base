 #pragma once

namespace cgb
{

	// Forward declare templated cgb::buffer and set up some type defs.
	// The definition of the buffer will always be inside the context-specific implementation files.
	//template <typename Cfg>
	//struct buffer_t;

	using generic_buffer_t = buffer_t<generic_buffer_data>;
	using uniform_buffer_t = buffer_t<uniform_buffer_data>;
	using uniform_texel_buffer_t = buffer_t<uniform_texel_buffer_data>;
	using storage_buffer_t = buffer_t<storage_buffer_data>;
	using storage_texel_buffer_t = buffer_t<storage_texel_buffer_data>;
	using vertex_buffer_t = buffer_t<vertex_buffer_data>;
	using index_buffer_t = buffer_t<index_buffer_data>;

	using unique_generic_buffer_t = std::unique_ptr<buffer_t<generic_buffer_data>>;
	using unique_uniform_buffer_t = std::unique_ptr<buffer_t<uniform_buffer_data>>;
	using unique_uniform_texel_buffer_t = std::unique_ptr<buffer_t<uniform_texel_buffer_data>>;
	using unique_storage_buffer_t = std::unique_ptr<buffer_t<storage_buffer_data>>;
	using unique_storage_texel_buffer_t = std::unique_ptr<buffer_t<storage_texel_buffer_data>>;
	using unique_vertex_buffer_t = std::unique_ptr<buffer_t<vertex_buffer_data>>;
	using unique_index_buffer_t = std::unique_ptr<buffer_t<index_buffer_data>>;

	using shared_generic_buffer_t = std::shared_ptr<buffer_t<generic_buffer_data>>;
	using shared_uniform_buffer_t = std::shared_ptr<buffer_t<uniform_buffer_data>>;
	using shared_uniform_texel_buffer_t = std::shared_ptr<buffer_t<uniform_texel_buffer_data>>;
	using shared_storage_buffer_t = std::shared_ptr<buffer_t<storage_buffer_data>>;
	using shared_storage_texel_buffer_t = std::shared_ptr<buffer_t<storage_texel_buffer_data>>;
	using shared_vertex_buffer_t = std::shared_ptr<buffer_t<vertex_buffer_data>>;
	using shared_index_buffer_t = std::shared_ptr<buffer_t<index_buffer_data>>;

	using generic_buffer		= std::variant<generic_buffer_t, unique_generic_buffer_t, shared_generic_buffer_t>;
	using uniform_buffer		= std::variant<uniform_buffer_t, unique_uniform_buffer_t, shared_uniform_buffer_t>;
	using uniform_texel_buffer	= std::variant<uniform_texel_buffer_t, unique_uniform_texel_buffer_t, shared_uniform_texel_buffer_t>;
	using storage_buffer		= std::variant<storage_buffer_t, unique_storage_buffer_t, shared_storage_buffer_t>;
	using storage_texel_buffer	= std::variant<storage_texel_buffer_t, unique_storage_texel_buffer_t, shared_storage_texel_buffer_t>;
	using vertex_buffer			= std::variant<vertex_buffer_t, unique_vertex_buffer_t, shared_vertex_buffer_t>;
	using index_buffer			= std::variant<index_buffer_t, unique_index_buffer_t, shared_index_buffer_t>;

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
