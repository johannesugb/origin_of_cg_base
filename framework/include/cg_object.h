#pragma once

namespace cgb
{
	/**	\brief Override this base class for objects to update and/or render
	 * 
	 *	Base class for any object which should be updated or rendered
	 *	during a \ref run. A specific object should override all the 
	 *	virtual methods which it would like to hook-into with custom
	 *	functionality, just leave the other virtual methods in their
	 *	empty default implementation.
	 *
	 *	Invocation order of cg_object's methods:
	 *	  1. initialize
	 *	  loop:
	 *	    2. fixed_update 
	 *	       possibly continue; // depending on the timer_interface used
	 *	    3. update
	 *	    4. render
	 *	    5. render_gizmos
	 *	    6. render_gui
	 *	  loop-end
	 *	  7. finalize
	 */
	class cg_object
	{
	public:
		/**
		 * \brief Constructor which automatically generates a name for this object
		 */
		cg_object() :
			mName("cg_object #" + std::to_string(sGeneratedNameId++))
		{
		}

		/**	\brief Constructor
		 *	\param pName Name by which this object can be identified
		 */
		cg_object(std::string pName) : 
			mName(pName)
		{
		}

		/** Returns the name of this cg_object */
		const std::string& name() const { return mName; }

		/**	\brief Initialize this cg_object
		 *
		 *	This is the first method in the lifecycle of a cg_object,
		 *	that is invoked by the framework during a \ref run. 
		 *	It is called only once.
		 */
		virtual void initialize() {}

		/**	\brief Update this cg_object at fixed intervals
		 *
		 *	This method is called at fixed intervals with a fixed 
		 *	delta time. Please note that this only applies to timers
		 *	which support fixed update intervals. For all other timers,
		 *	this method will simply be called each frame before the 
		 *	regular \ref update is called.
		 */
		virtual void fixed_update() {}

		/**	\brief Update this cg_object before rendering with varying delta time
		 *
		 *	This method is called at varying intervals. Query the
		 *	timer_interface's delta time to get the time which has passed since
		 *	the last frame. This method will always be called after
		 *	\ref fixed_update and before \ref render.
		 */
		virtual void update() {}

		/**	\brief Render this cg_object 
		 *
		 *	This method is called whenever this cg_object should
		 *	perform its rendering tasks. It is called right after
		 *	all the \ref update methods have been invoked.
		 */
		virtual void render() {}

		/**	\brief Render gizmos for this cg_object
		 *
		 *	This method can be used to render additional information
		 *	about this cg_object like, for instance, debug information.
		 *	This method will always be called after all \ref render
		 *	methods of the current \ref run have been invoked.
		 */
		virtual void render_gizmos() {}

		/**	\brief Render the GUI for this cg_object
		 *
		 *	Use this method to render the graphical user interface.
		 *	This method will always be called after all \ref render_gizmos
		 *	methods of the current \ref run have been invoked.
		 */
		virtual void render_gui() {}

		/**	\brief Cleanup this cg_object
		 *
		 *	This is the last method in the lifecycle of a cg_object,
		 *	that is invoked by the framework during a \ref run.
		 *	It is called only once.
		 */
		virtual void finalize() {}

	private:
		inline static int32_t sGeneratedNameId = 0;
		std::string mName;
	};
}
