#pragma once

namespace cgb
{
	/**	\brief Basic timer which only supports time steps with varying delta time
	 *
	 *	This kind of timer leads to as many updates/renders as possible.
	 *	Beware that there is no fixed time step and, thus, no @ref fixed_delta_time
	 *	Time between frames is always varying.
	 */
	class varying_update_only_timer : public timer
	{
	public:
		varying_update_only_timer();

		timer_frame_type tick();

		float absolute_time() const override;
		float frame_time() const override;
		float fixed_delta_time() const override;
		float delta_time() const override;
		float time_scale() const override;
		double precise_absolute_time() const override;
		double precise_frame_time() const override;
		double precise_fixed_delta_time() const override;
		double precise_delta_time() const override;
		double precise_time_scale() const override;

	private:
		double mStartTime;
		double mAbsTime;
		double mTimeSinceStart;
		double mLastTime;
		double mDeltaTime;
	};
}
