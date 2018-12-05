#include "varying_update_only_timer.h"

namespace cgb
{
	varying_update_only_timer::varying_update_only_timer()
		: mStartTime(0.0),
		mLastTime(0.0),
		mAbsTime(0.0),
		mTimeSinceStart(0.0),
		mDeltaTime(0.0)
	{
		mAbsTime = mStartTime = context().get_time();
	}

	timer_frame_type varying_update_only_timer::tick()
	{
		mAbsTime = glfwGetTime();
		mTimeSinceStart = mAbsTime - mStartTime;
		mDeltaTime = mTimeSinceStart - mLastTime;
		mLastTime = mTimeSinceStart;
		return timer_frame_type::any;
	}

	float varying_update_only_timer::absolute_time() const
	{
		return static_cast<float>(mAbsTime);
	}

	float varying_update_only_timer::frame_time() const
	{
		return static_cast<float>(mTimeSinceStart);
	}

	float varying_update_only_timer::fixed_delta_time() const
	{
		return static_cast<float>(mDeltaTime);
	}

	float varying_update_only_timer::delta_time() const
	{
		return static_cast<float>(mDeltaTime);
	}

	float varying_update_only_timer::time_scale() const
	{
		return 1.0f;
	}

	double varying_update_only_timer::precise_absolute_time() const
	{
		return mAbsTime;
	}

	double varying_update_only_timer::precise_frame_time() const
	{
		return mTimeSinceStart;
	}

	double varying_update_only_timer::precise_fixed_delta_time() const
	{
		return mDeltaTime;
	}

	double varying_update_only_timer::precise_delta_time() const
	{
		return mDeltaTime;
	}

	double varying_update_only_timer::precise_time_scale() const
	{
		return 1.0;
	}

}
