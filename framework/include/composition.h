#pragma once

namespace cgb
{
	/**	A composition brings together all of the separate components, which there are
	 *	 - A timer
	 *	 - One or more windows
	 *	 - One or more @ref cg_element(-derived objects)
	 *	 
	 *	Upon @ref start, a composition spins up the game-/rendering-loop in which
	 *	all of the @ref cg_element's methods are called.
	 *	
	 *	A composition will internally call @ref set_global_composition_data in order
	 *	to make itself the currently active composition. By design, there can only 
	 *	be one active composition_interface at at time. You can think of a composition
	 *	being something like a scene, of which typically one can be active at any 
	 *	given point in time.
	 *	
	 *	\remark You don't HAVE to use this composition-class, if you are developing 
	 *	an alternative composition class or a different approach and still want to use
	 *	a similar structure as proposed by this composition-class, please make sure 
	 *	to call @ref set_global_composition_data 
	 */
	template <typename TTimer, typename TExecutor>
	class composition : public composition_interface
	{
	public:
		composition() :
			mWindows(),
			mElements(),
			mTimer(),
			mExecutor(this),
			mInputBuffers(),
			mInputBufferUpdateIndex(0),
			mInputBufferConsumerIndex(1),
			mShouldStop(false),
			mShouldSwapInputBuffers(false),
			mInputBufferGoodToGo(true),
			mIsRunning(false)
		{
		}

		composition(std::initializer_list<window*> pWindows, std::initializer_list<cg_element*> pObjects) :
			mWindows(pWindows),
			mElements(pObjects),
			mTimer(),
			mExecutor(this),
			mInputBuffers(),
			mInputBufferUpdateIndex(0),
			mInputBufferConsumerIndex(1),
			mShouldStop(false),
			mShouldSwapInputBuffers(false),
			mInputBufferGoodToGo(true),
			mIsRunning(false)
		{
		}

		/** Provides access to the timer which is used by this composition */
		timer_interface& time() override
		{
			return mTimer;
		}

		/** Provides to the currently active input buffer, which contains the
		 *	current user input data */
		input_buffer& input() override
		{
			return mInputBuffers[mInputBufferConsumerIndex];
		}

		/** Returns the @ref cg_element at the given index */
		cg_element* element_at_index(size_t pIndex) override
		{
			if (pIndex < mElements.size())
				return mElements[pIndex];

			return nullptr;
		}

		/** Finds a @ref cg_element by its name 
		 *	\returns The element found or nullptr
		 */
		cg_element* element_by_name(const std::string& pName) override
		{
			auto found = std::find_if(
				std::begin(mElements), 
				std::end(mElements), 
				[&pName](const cg_element* element)
				{
					return element->name() == pName;
				});

			if (found != mElements.end())
				return *found;

			return nullptr;
		}

		/** Finds the @ref cg_element(s) with matching type.
		 *	@param pType	The type to look for
		 *	@param pIndex	Use this parameter to get the n-th element of the given type
		 *	\returns An element of the given type or nullptr
		 */
		cg_element* element_by_type(const std::type_info& pType, uint32_t pIndex) override
		{
			uint32_t nth = 0;
			for (auto* element : mElements)
			{
				if (typeid(element) == pType)
				{
					if (pIndex == nth++)
					{
						return element;
					}
				}
			}
			return nullptr;
		}

		/** Add all elements which are about to be added to the composition */
		void add_pending_elements()
		{
			// Make a copy of all the elements to be added to not interfere with erase-operations:
			auto toBeAdded = mElementsToBeAdded;
			for (auto el : toBeAdded) {
				add_element_immediately(*el);
			}
			assert(mElementsToBeAdded.size() == 0);
		}

		/** Remove all elements which are about to be removed */
		void remove_pending_elements()
		{
			// Make a copy of all the elements to be added to not interfere with erase-operations:
			auto toBeRemoved = mElementsToBeRemoved;
			for (auto el : toBeRemoved) {
				remove_element_immediately(*el);
			}
			assert(mElementsToBeRemoved.size() == 0);
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
					LOG_WARNING(fmt::format("More than {} iterations in spin-lock", i+1));
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
				thiz->add_pending_elements();

				// signal context
				cgb::context().begin_frame();

				frameType = thiz->mTimer.tick();

				wait_for_input_buffers_swapped(thiz);

				// 2. fixed_update
				if ((frameType & timer_frame_type::fixed) != timer_frame_type::none)
				{
					thiz->mExecutor.execute_fixed_updates(thiz->mElements);
				}

				if ((frameType & timer_frame_type::varying) != timer_frame_type::none)
				{
					// 3. update
					thiz->mExecutor.execute_updates(thiz->mElements);

					// Tell the main thread that we'd like to have the new input buffers from A) here:
					please_swap_input_buffers(thiz);

					// 4. render
					thiz->mExecutor.execute_renders(thiz->mElements);

					// 5. render_gizmos
					thiz->mExecutor.execute_render_gizmos(thiz->mElements);
					
					// 6. render_gui
					thiz->mExecutor.execute_render_guis(thiz->mElements);
				}
				else
				{
					// If not done from inside the positive if-branch, tell the main thread of our input buffer update desire here:
					please_swap_input_buffers(thiz);
				}

				// signal context
				cgb::context().end_frame();

				thiz->remove_pending_elements();
			}

		}

	public:
		void add_element(cg_element& pElement) override
		{
			mElementsToBeAdded.push_back(&pElement);
		}

		void add_element_immediately(cg_element& pElement) override
		{
			mElements.push_back(&pElement);
			pElement.initialize();
			// Remove from mElementsToBeAdded container (if it was contained in it)
			mElementsToBeAdded.erase(std::remove(std::begin(mElementsToBeAdded), std::end(mElementsToBeAdded), &pElement));
		}

		void remove_element(cg_element& pElement) override
		{
			mElementsToBeRemoved.push_back(&pElement);
		}

		void remove_element_immediately(cg_element& pElement, bool pIsBeingDestructed = false) override
		{
			if (!pIsBeingDestructed) {
				assert(std::find(std::begin(mElements), std::end(mElements), &pElement) != mElements.end());
				pElement.finalize();
				// Remove from the actual elements-container
				mElements.erase(std::remove(std::begin(mElements), std::end(mElements), &pElement));
				// ...and from mElementsToBeRemoved
				mElementsToBeRemoved.erase(std::remove(std::begin(mElementsToBeRemoved), std::end(mElementsToBeRemoved), &pElement));
			}
			else {
				LOG_DEBUG_EM(fmt::format("Removing element with name[{}] and address[{}] issued from cg_element's destructor",
										 pElement.name(),
										 fmt::ptr(&pElement)));
			}
		}

		window* window_in_focus() override
		{
			if (mWindows.size() > 0) {
				return mWindows[0];
			}
			return nullptr;
		}

		window* window_by_name(const std::string& pName) override
		{
			auto it = std::find_if(std::begin(mWindows), std::end(mWindows),
								   [&pName](const window* w) {
									   return w->name() == pName;
								   });
			return it != mWindows.end() ? *it : nullptr;
		}

		window* window_by_id(uint32_t pId) override
		{
			auto it = std::find_if(std::begin(mWindows), std::end(mWindows),
								   [&pId](const window* w) {
									   return w->id() == pId;
								   });
			return it != mWindows.end() ? *it : nullptr;
		}

		void start() override
		{
			// Make myself the current composition_interface
			composition_interface::set_current(this);

			// 1. initialize
			for (auto& o : mElements)
			{
				o->initialize();
			}

			// Signal context after initialization
			cgb::context().begin_composition();

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

			// Signal context before finalization
			cgb::context().end_composition();

			// 7. finalize
			for (auto& o : mElements)
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
		std::vector<cg_element*> mElements;
		std::vector<cg_element*> mElementsToBeAdded;
		std::vector<cg_element*> mElementsToBeRemoved;
		TTimer mTimer;
		TExecutor mExecutor;
		std::array<input_buffer, 2> mInputBuffers;
		int32_t mInputBufferUpdateIndex;
		int32_t mInputBufferConsumerIndex;
		std::atomic_bool mShouldStop;
		std::atomic_bool mShouldSwapInputBuffers;
		std::atomic_bool mInputBufferGoodToGo;
		bool mIsRunning;
	};
}
