#pragma once

namespace cgb
{
	/**	\brief Timer enabling a fixed update rate
	 *
	 *	This timer has a fixed simulation rate (i.e. constant fixed_delta_time)
	 *  and variable rendering rate (i.e. as many render fps as possible, with
	 *  variable rendering delta time). It also provides a minimum 
	 *  render-fps functionality (use @ref set_min_render_hz)
	 */
	class fixed_update_timer : public timer
	{
	public:
		fixed_update_timer();

		timer_frame_type tick();

		void set_min_render_hertz(double pMinRenderHz);
		void set_fixed_simulation_hertz(double pFixedSimulationHz);

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
		double mCurrentRenderHz;
		double mMinRenderHz;
		double mMaxRenderDeltaTime;

		double mFixedDeltaTime;
		double mFixedHz;
		double mLastFixedTick;
		double mNextFixedTick;
	};
}
