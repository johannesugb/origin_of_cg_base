#pragma once

namespace cgb
{
	class QuakeCamera : public Camera, public cg_element
	{
	public:
		QuakeCamera();
		~QuakeCamera();

		void HandleInputOnly();
		void initialize() override;
		void finalize() override;
		void update() override;

		float rotation_speed() const { return mRotationSpeed; }
		float move_speed() const { return mMoveSpeed; }
		float fast_multiplier() const { return mFastMultiplier; }
		float slow_multiplier() const { return mSlowMultiplier; }

		void set_rotation_speed(float value) { mRotationSpeed = value; }
		void set_move_speed(float value) { mMoveSpeed = value; }
		void set_fast_multiplier(float value) { mFastMultiplier = value; }
		void set_slow_multiplier(float value) { mSlowMultiplier = value; }

	private:
		float mRotationSpeed;
		float mMoveSpeed;
		float mFastMultiplier;
		float mSlowMultiplier;

		glm::vec2 mAccumulatedMouseMovement;
		bool mCaptureInput;

		void AddToCameraPositionRelative(const glm::vec4& homoVectorToAdd, double deltaTime);
		void AddToCameraPositionAbsolute(const glm::vec4& homoVectorToAdd, double deltaTime);

	};
}
