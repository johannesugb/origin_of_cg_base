#pragma once

namespace cgb
{
	template <typename TTimer>
	class composition_with_timer : public composition
	{
	public:
		composition_with_timer() :
			mWindows(),
			mObjects(),
			mTimer(),
			mShouldStop(false),
			mIsRunning(false)
		{
		}

		composition_with_timer(std::initializer_list<window*> pWindows, std::initializer_list<std::shared_ptr<cg_object>> pObjects) :
			mWindows(pWindows),
			mObjects(pObjects),
			mTimer(),
			mShouldStop(false),
			mIsRunning(false)
		{
		}

		cgb::timer& time() override
		{
			return static_cast<cgb::timer&>(mTimer);
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

		void start() override
		{
			// Make myself the current composition
			composition::set_current(this);

			// Used to distinguish between "simulation" and "render"-frames
			auto frameType = timer_frame_type::none;

			// 1. initialize
			for (auto& o : mObjects)
			{
				o->initialize();
			}

			int tmp_tmp_tmp = 1;


			// game/render/composition-loop:
			mIsRunning = true;
			while (!mShouldStop)
			{
				frameType = mTimer.tick();
				
				// 2. fixed_update
				if ((frameType & timer_frame_type::fixed) != timer_frame_type::none)
				{
					LOG_INFO("fixed frame with fixed delta-time[%f]", time().fixed_delta_time());
					for (auto& o : mObjects)
					{
						o->fixed_update();
					}
				}

				if ((frameType & timer_frame_type::varying) != timer_frame_type::none)
				{
					LOG_INFO("varying frame with delta-time[%f]", time().delta_time());

					// 3. update
					for (auto& o : mObjects)
					{
						o->update();
					}
					
					// 4. render
					for (auto& o : mObjects)
					{
						o->render();
					}

					// 5. render_gizmos
					for (auto& o : mObjects)
					{
						o->render_gizmos();
					}

					// 6. render_gui
					for (auto& o : mObjects)
					{
						o->render_gui();
					}
				}

				for (int i = 0; i < 1000000; ++i);
				if (tmp_tmp_tmp++ > 30) 
					mShouldStop = true;
			}
			mIsRunning = false;

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
		bool mShouldStop;
		bool mIsRunning;
	};
}
