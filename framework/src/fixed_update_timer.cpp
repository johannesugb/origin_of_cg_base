#include "fixed_update_timer.h"

namespace cgb
{
	fixed_update_timer::fixed_update_timer() :
		mTimeSinceStart(0.0),
		mLastTime(0.0),
		mDeltaTime(0.0),
		mMinRenderHz(1.0),
		mMaxRenderDeltaTime(1.0 / 1.0),
		mCurrentRenderHz(0.0),
		mFixedHz(60.0),
		mFixedDeltaTime(1.0 / 60.0)
	{
		mLastFixedTick = mAbsTime = mStartTime = context().get_time();
		mNextFixedTick = mLastFixedTick + mFixedDeltaTime;
	}

	timer_frame_type fixed_update_timer::tick()
	{
		mAbsTime = context().get_time();
		mTimeSinceStart = mAbsTime - mStartTime;

		auto dt = mTimeSinceStart - mLastTime;

		// should we simulate or render?
		if (mAbsTime > mNextFixedTick && dt < mMaxRenderDeltaTime)
		{
			mLastFixedTick = mNextFixedTick;
			mNextFixedTick += mFixedDeltaTime;
			return timer_frame_type::fixed;
		}

		mDeltaTime = dt;
		mLastTime = mTimeSinceStart;
		mCurrentRenderHz = 1.0 / mDeltaTime;
		return timer_frame_type::varying;
	}

	void fixed_update_timer::set_min_render_hertz(double pMinRenderHz)
	{
		mMinRenderHz = pMinRenderHz;
		mMaxRenderDeltaTime = 1.0 / mMinRenderHz;
	}

	void fixed_update_timer::set_fixed_simulation_hertz(double pFixedSimulationHz)
	{
		mFixedHz = pFixedSimulationHz;
		mFixedDeltaTime = 1.0 / mFixedHz;
	}

	float fixed_update_timer::absolute_time() const
	{
		return static_cast<float>(mAbsTime);
	}

	float fixed_update_timer::time_since_start() const
	{
		return static_cast<float>(mTimeSinceStart);
	}

	float fixed_update_timer::fixed_delta_time() const
	{
		return static_cast<float>(mFixedDeltaTime);
	}

	float fixed_update_timer::delta_time() const
	{
		return static_cast<float>(mDeltaTime);
	}

	float fixed_update_timer::time_scale() const
	{
		return 1.0f;
	}

	double fixed_update_timer::precise_absolute_time() const
	{
		return mAbsTime;
	}

	double fixed_update_timer::precise_frame_time() const
	{
		return mTimeSinceStart;
	}

	double fixed_update_timer::precise_fixed_delta_time() const
	{
		return mFixedDeltaTime;
	}

	double fixed_update_timer::precise_delta_time() const
	{
		return mDeltaTime;
	}

	double fixed_update_timer::precise_time_scale() const
	{
		return 1.0;
	}
}
