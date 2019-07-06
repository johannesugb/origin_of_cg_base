#pragma once

namespace cgb
{
	/** Gets a reference to the data stored in a variant, regardless of how it is stored/referenced there,
	*	be it stored directly or referenced via a smart pointer.
	*/
	template <typename T, typename V>
	T& get(V& v)	{
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

	/** Gets a reference to the data stored in a variant, regardless of how it is stored/referenced there,
	*	be it stored directly or referenced via a smart pointer.
	*/
	template <typename T, typename V>
	const T& get(const V& v)	{
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

	/** Transform an element into a unique_ptr by moving from it.
	 *	The parameter must be an r-value reference and will not be usable 
	 *	after this function call anymore because it has been moved from.
	 *
	 *	Example:
	 *    `buffer b = cgb::make_unique(cgb::create(cgb::vertex_buffer{1,2}, ...));`
	 */
	template <typename T>
	std::unique_ptr<T> make_unique(T&& t)
	{
		return std::make_unique<T>(std::move(t));
	}

	/** Transform an element into a shared_ptr by moving from it.
	*	The parameter must be an r-value reference and will not be usable 
	*	after this function call anymore because it has been moved from.
	*
	*	Example:
	*    `buffer b = cgb::make_shared(cgb::create(cgb::vertex_buffer{1,2}, ...));`
	*/
	template <typename T>
	std::shared_ptr<T> make_shared(T&& t)
	{
		return std::make_shared<T>(std::move(t));
	}

}
