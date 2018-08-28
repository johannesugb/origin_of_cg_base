#pragma once

namespace cgb
{
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

		cgb::timer_interface& time() override
		{
			return static_cast<cgb::timer_interface&>(mTimer);
		}

		cgb::input_buffer& input() override
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

		static void please_swap_input_buffers(composition* thiz)
		{
			thiz->mShouldSwapInputBuffers = true;
			thiz->mInputBufferGoodToGo = false;
			glfwPostEmptyEvent();
		}

		static void wait_for_input_buffers_swapped(composition* thiz)
		{
			while (!thiz->mInputBufferGoodToGo)
				LOG_VERBOSE("input buffer spin lock");
			assert(thiz->mShouldSwapInputBuffers == false);
		}

		static void render_thread(composition* thiz)
		{
			// Used to distinguish between "simulation" and "render"-frames
			auto frameType = timer_frame_type::none;
			int tmp_tmp_tmp = 1;

			while (!thiz->mShouldStop)
			{
				frameType = thiz->mTimer.tick();

				wait_for_input_buffers_swapped(thiz);

				// 2. fixed_update
				if ((frameType & timer_frame_type::fixed) != timer_frame_type::none)
				{
					LOG_INFO("fixed frame with fixed delta-time[%f]", composition_interface::current()->time().fixed_delta_time());
					for (auto& o : thiz->mObjects)
					{
						o->fixed_update();
					}
				}

				if ((frameType & timer_frame_type::varying) != timer_frame_type::none)
				{
					LOG_INFO("varying frame with delta-time[%f]", composition_interface::current()->time().delta_time());

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

				for (int i = 0; i < 1000000; ++i);
				if (tmp_tmp_tmp++ > 30)
					thiz->mShouldStop = true;
			}
		}

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
					std::swap(mInputBufferUpdateIndex, mInputBufferUpdateIndex);
					mInputBuffers[mInputBufferUpdateIndex].reset();
					context().change_target_input_buffer(mInputBuffers[mInputBufferUpdateIndex]);
				}

				glfwWaitEvents();
			}

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

		void stop() override
		{
			mShouldStop = true;
		}

		bool is_running() override
		{
			return mIsRunning;
		}

	private:
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
