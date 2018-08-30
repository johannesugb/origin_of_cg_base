#pragma once

namespace cgb
{
#pragma region global data representing the currently active composition
	/**	\brief Get the current timer, which represents the current game-/render-time 
	 *	\remark This is just a shortcut to \ref composition_interface::current()->time();
	 */
	inline timer_interface& time() {
		return composition_interface::current()->time();
	}

	/** \brief Get the current frame's input data 
	 *	\remark This is just a shortcut to \ref composition_interface::current()->input();
	 */
	inline input_buffer& input() {
		return composition_interface::current()->input();
	}

	/** \brief Get access to the currently active objects 
	 *	\remark This is just a shortcut to \ref *composition_interface::current();
	 */
	inline composition_interface& current_composition() {
		return *composition_interface::current();
	}
#pragma endregion 

	/**	A composition brings together all of the separate components, which there are
	 *	 - A timer
	 *	 - One or more windows
	 *	 - One or more \ref cg_object(-derived objects)
	 *	 
	 *	Upon \ref start, a composition spins up the game-/rendering-loop in which
	 *	all of the \ref cg_object's methods are called.
	 *	
	 *	A composition will internally call \ref set_global_composition_data in order
	 *	to make itself the currently active composition. By design, there can only 
	 *	be one active composition_interface at at time. You can think of a composition
	 *	being something like a scene, of which typically one can be active at any 
	 *	given point in time.
	 *	
	 *	\remark You don't HAVE to use this composition-class, if you are developing 
	 *	an alternative composition class or a different approach and still want to use
	 *	a similar structure as proposed by this composition-class, please make sure 
	 *	to call \ref set_global_composition_data 
	 */
	template <typename TTimer>
	class composition : public composition_interface
	{
	public:
		composition() :
			mWindows(),
			mObjects(),
			mTimer(),
			mInputBuffers(),
			mInputBufferUpdateIndex(0),
			mInputBufferConsumerIndex(1),
			mShouldStop(false),
			mShouldSwapInputBuffers(false),
			mInputBufferGoodToGo(true),
			mIsRunning(false)
		{
		}

		composition(std::initializer_list<window*> pWindows, std::initializer_list<std::shared_ptr<cg_object>> pObjects) :
			mWindows(pWindows),
			mObjects(pObjects),
			mTimer(),
			mInputBuffers(),
			mInputBufferUpdateIndex(0),
			mInputBufferConsumerIndex(1),
			mShouldStop(false),
			mShouldSwapInputBuffers(false),
			mInputBufferGoodToGo(true),
			mIsRunning(false)
		{
		}

		timer_interface& time() override
		{
			return mTimer;
		}

		input_buffer& input() override
		{
			return mInputBuffers[mInputBufferConsumerIndex];
		}

		cg_object* object_at_index(size_t pIndex) override
		{
			if (pIndex < mObjects.size())
				return mObjects[pIndex].get();
			return nullptr;
		}

		cg_object* object_by_name(const std::string& pName) override
		{
			auto found = std::find_if(
				std::begin(mObjects), 
				std::end(mObjects), 
				[&pName](const std::shared_ptr<cg_object>& element)
				{
					return element->name() == pName;
				});

			if (found != mObjects.end())
				return found->get();
			return nullptr;
		}

		cg_object* object_by_type(const std::type_info& pType, uint32_t pIndex) override
		{
			uint32_t nth = 0;
			for (auto& element : mObjects)
			{
				if (typeid(element.get()) == pType)
				{
					if (pIndex == nth++)
					{
						return element.get();
					}
				}
			}
			return nullptr;
		}

	private:
		/** Signal the main thread to start swapping input buffers */
		static void please_swap_input_buffers(composition* thiz)
		{
			thiz->mShouldSwapInputBuffers = true;
			thiz->mInputBufferGoodToGo = false;
			glfwPostEmptyEvent();
		}

		/** Signal the rendering thread that input buffers have been swapped */
		void have_swapped_input_buffers()
		{
			mShouldSwapInputBuffers = false;
			mInputBufferGoodToGo = true;
		}

		/** Wait on the rendering thread until the main thread has swapped the input buffers */
		static void wait_for_input_buffers_swapped(composition* thiz)
		{
			for (int i=0; !thiz->mInputBufferGoodToGo; ++i)
			{
				if ((i+1) % 1000 == 0)
				{
					LOG_WARNING("More than %d iterations in spin-lock", i+1);
				}
			}
			assert(thiz->mShouldSwapInputBuffers == false);
		}

		/** Rendering thread's main function */
		static void render_thread(composition* thiz)
		{
			// Used to distinguish between "simulation" and "render"-frames
			auto frameType = timer_frame_type::none;

			while (!thiz->mShouldStop)
			{
				frameType = thiz->mTimer.tick();

				wait_for_input_buffers_swapped(thiz);

				// 2. fixed_update
				if ((frameType & timer_frame_type::fixed) != timer_frame_type::none)
				{
					//LOG_INFO("fixed frame with fixed delta-time[%f]", composition_interface::current()->time().fixed_delta_time());
					for (auto& o : thiz->mObjects)
					{
						o->fixed_update();
					}
				}

				if ((frameType & timer_frame_type::varying) != timer_frame_type::none)
				{
					//LOG_INFO("varying frame with delta-time[%f]", composition_interface::current()->time().delta_time());

					// 3. update
					for (auto& o : thiz->mObjects)
					{
						o->update();
					}

					// Tell the main thread that we'd like to have the new input buffers from A) here:
					please_swap_input_buffers(thiz);

					// 4. render
					for (auto& o : thiz->mObjects)
					{
						o->render();
					}

					// 5. render_gizmos
					for (auto& o : thiz->mObjects)
					{
						o->render_gizmos();
					}

					// 6. render_gui
					for (auto& o : thiz->mObjects)
					{
						o->render_gui();
					}
				}
				else
				{
					// Or if not from 'A)', tell the main thread for our input buffer update desire from B) here:
					please_swap_input_buffers(thiz);
				}
			}
		}

	public:
		/** Start a game/rendering-loop for this composition_interface,
		 *	This will also spawn a separate rendering thread.
		 *	The main thread will mainly focus on processing input.
		 */
		void start() override
		{
			// Make myself the current composition_interface
			composition_interface::set_current(this);

			// 1. initialize
			for (auto& o : mObjects)
			{
				o->initialize();
			}

			// Enable receiving input
			for (const auto& window : mWindows)
			{
				// Write into the buffer at mInputBufferUpdateIndex,
				// let client-objects read from the buffer at mInputBufferConsumerIndex
				context().start_receiving_input_from_window(*window, mInputBuffers[mInputBufferUpdateIndex]);
			}

			// game-/render-loop:
			mIsRunning = true;

			// off it goes
			std::thread renderThread(render_thread, this);
			
			while (!mShouldStop)
			{
				if (mShouldSwapInputBuffers)
				{
					std::swap(mInputBufferUpdateIndex, mInputBufferConsumerIndex);
					mInputBuffers[mInputBufferUpdateIndex].prepare_for_next_frame(mInputBuffers[mInputBufferConsumerIndex]);
					context().change_target_input_buffer(mInputBuffers[mInputBufferUpdateIndex]);
					have_swapped_input_buffers();
				}

				glfwWaitEvents();
			}

			renderThread.join();

			mIsRunning = false;

			// Stop the input
			for (const auto& window : mWindows)
			{
				context().stop_receiving_input_from_window(*window);
			}

			// 7. finalize
			for (auto& o : mObjects)
			{
				o->finalize();
			}

		}

		/** Stop a currently running game/rendering-loop for this composition_interface */
		void stop() override
		{
			mShouldStop = true;
		}

		/** True if this composition_interface has been started but not yet stopped or finished. */
		bool is_running() override
		{
			return mIsRunning;
		}

	private:
		static composition* sComposition;
		std::vector<window*> mWindows;
		std::vector<std::shared_ptr<cg_object>> mObjects;
		TTimer mTimer;
		std::array<input_buffer, 2> mInputBuffers;
		int32_t mInputBufferUpdateIndex;
		int32_t mInputBufferConsumerIndex;
		std::atomic_bool mShouldStop;
		std::atomic_bool mShouldSwapInputBuffers;
		std::atomic_bool mInputBufferGoodToGo;
		bool mIsRunning;
	};
}
