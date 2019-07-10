#pragma once

namespace cgb
{
	/** Intended to be used to sync access to context() calls.
	 *	I.e. if you are using a parallel executor, wrap all calls to 
	 *	the context in the SYNC_CALL macro or use the SYNC_THIS_SCOPE macro!
	 */
	extern std::mutex gContextSyncMutex;

#define SYNC_THIS_SCOPE std::scoped_lock<std::mutex> guard(cgb::gContextSyncMutex);
#define SYNC_CALL(x) do { SYNC_THIS_SCOPE x; } while(false)


	/** Calls the context's @ref check_error method, passing the current file and line to it. */
#define CHECK_ERROR() context().check_error(__FILE__, __LINE__);

	/** A struct holding a context-specific function
	*	Create such a struct and use it in conjunction with the macros SET_VULKAN_FUNCTION and 
	*	SET_OPENGL46_FUNCTION to specify context-specific code.
	*
	*	Example: Create a context-specific function to select a format depending on the context used:
	*
	*	 auto selectImageFormat = cgb::context_specific_function<cgb::image_format()>{}
	*								.VK_FUNC([]() { return cgb::image_format{ vk::Format::eR8G8B8Unorm }; })
	*								.GL_FUNC([]() { return cgb::image_format{ GL_RGB }; });
	*/
	template<typename T>
	struct context_specific_function
	{
		std::function<T> mFunction;

		context_specific_function() {}

		template <typename F>
		context_specific_function(F func)
		{
			set_function(std::move(func));
		}

		template <typename F>
		auto& operator==(F func)
		{
			set_function(std::move(func));
			return *this;
		}

		/** Sets the function. However, this is not intended to be called directly!
		*	Use the macros VK_FUNC and GL_FUNC instead!
		*	Use those macros exactly in the same way as you would call this function;
		*	i.e. instead of .set_function(...) just write .VK_FUNC(...) for the Vulkan context.
		*/
		auto& set_function(std::function<T> func)
		{
			mFunction = std::move(func);
			return *this;
		}

		/** Function which is used by the macros SET_VULKAN_FUNCTION and SET_OPENGL46_FUNCTION.
		*	This is not intended to be called directly, so just use the macros!
		*/
		auto& do_nothing()
		{
			return *this;
		}
	};

#if defined(USE_OPENGL46_CONTEXT)
#define GL_ONLY(x) x
#define VK_ONLY(x) 
#define GL_FUNC(x) set_function(x)
#define VK_FUNC(x) do_nothing()
#elif defined(USE_VULKAN_CONTEXT)
#define GL_ONLY(x)
#define VK_ONLY(x) x
#define GL_FUNC(x) do_nothing()
#define VK_FUNC(x) set_function(x)
#endif


	/** Gets a reference to the data stored in a variant, regardless of how it is stored/referenced there,
	*	be it stored directly or referenced via a smart pointer.
	*/
	template <typename T, typename V>
	T& get(V& v)	
	{
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
	const T& get(const V& v)	
	{
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
	template <typename T>
	T& get(std::variant<T, std::unique_ptr<T>, std::shared_ptr<T>>& v)	
	{
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
	template <typename T>
	const T& get(const std::variant<T, std::unique_ptr<T>, std::shared_ptr<T>>& v)	
	{
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

	// SFINAE test for detecting if a type has a `.size()` member
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

	// SFINAE test for detecting if a type does NOT have a `.size()` member
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

	template<typename T, typename E> 
	typename std::enable_if<has_size_member<T>::value, const E&>::type first_or_only_element(const T& t) {
		return t[0];
	}

	template<typename T> 
	typename std::enable_if<has_no_size_member<T>::value, const T&>::type first_or_only_element(const T& t) {
		return t;
	}

}
