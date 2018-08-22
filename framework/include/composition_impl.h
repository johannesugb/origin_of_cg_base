#pragma once

namespace cgb
{
	template <typename TTimer>
	class composition_impl : public composition
	{
	public:
		composition_impl() :
			mObjects(),
			mTimer(),
			mShouldStop(false)
		{
		}

		cgb::time& time() override
		{
			return static_cast<cgb::time&>(mTimer);
		}

		cg_object* object_at_index(size_t pIndex) override
		{
			if (pIndex < mObjects.size())
				return mObjects[pIndex].get();
			return nullptr;
		}

		cg_object* object_by_name(std::string pName) override
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

		void start() override
		{
			auto frameType = timer_frame_type::none;

			// 1. initialize
			for (auto& o : mObjects)
			{
				o->initialize();
			}

			// game/render/composition-loop:
			while (!mShouldStop)
			{
				frameType = mTimer.tick();
				
				// 2. fixed_update
				if ((frameType & timer_frame_type::fixed) != timer_frame_type::none)
				{
					for (auto& o : mObjects)
					{
						o->fixed_update();
					}
				}

				if ((frameType & timer_frame_type::varying) != timer_frame_type::none)
				{
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

	private:
		std::vector<std::shared_ptr<cg_object>> mObjects;
		TTimer mTimer;
		bool mShouldStop;
	};
}
