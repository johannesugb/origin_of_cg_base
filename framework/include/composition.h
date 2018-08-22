#pragma once

namespace cgb
{
	class composition
	{
	public:
		/** \brief Get the currently active composition
		 *
		 *	By design, there can only be one active composition at at time.
		 *	You can think of composition being something like scenes, of 
		 *	which typically one can be active at any given point in time.
		 */
		static composition* current() { return sCurrentComposition; }

		/** Interface to the current game time */
		virtual timer& time() = 0;

		/** \brief Get the @ref cg_object at the given index
		 *
		 *	Get the @ref cg_object in this composition's objects-container at 
		 *	the given index. If the index is out of bounds, nullptr will be
		 *	returned.
		 */
		virtual cg_object* object_at_index(size_t) = 0;

		/** \brief Find a @ref cg_object by name
		 *
		 *	Find a @ref cg_object assigned to this composition by name.
		 *	If no object with the given name could be found, nullptr is returned.
		 */
		virtual cg_object* object_by_name(const std::string&) = 0;

		/** \brief Find an object by type 
		 *	\param pType type-identifier of the @ref cg_object to be searched for
		 *	\param pIndex Get the n-th object of the specified type
		 */
		virtual cg_object* object_by_type(const std::type_info& pType, uint32_t pIndex = 0) = 0;

		/** \brief Start a game/rendering-loop for this composition 
		 *
		 *	Attention: In subclasses of @ref composition, do not forget to call
		 *	@ref set_current no later than at the very beginning of this methods's
		 *	implementation!
		 */
		virtual void start() = 0;

		/** Stop a currently running game/rendering-loop for this composition */
		virtual void stop() = 0;

		/** True if this composition has been started but not yet stopped or finished. */
		virtual bool is_running() = 0;

	protected:
		/** \brief Set a new current composition 
		 *
		 *	Set a new composition. Remember: There can only be one
		 *	at any given point in time. This call will fail if a different
		 *	composition is currently set with @ref is_running() evaluating
		 *	to true.
		 */
		static void set_current(composition* pNewComposition)
		{
			if (nullptr != sCurrentComposition && sCurrentComposition->is_running())
			{
				throw std::runtime_error("There is already an active composition which is still running.");
			}
			// It's okay.
			sCurrentComposition = pNewComposition;
		}

	private:
		/** The (single) currently active composition */
		static composition* sCurrentComposition;
	};
}
