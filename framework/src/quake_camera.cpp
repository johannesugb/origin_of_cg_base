#include "quake_camera.h"

namespace cgb
{
	QuakeCamera::QuakeCamera() 
		: mRotationSpeed(0.001f)
		, mMoveSpeed(4.5f) // 4.5 m/s
		, mFastMultiplier(6.0f) // 27 m/s
		, mSlowMultiplier(0.2f) // 0.9 m/s
		, mAccumulatedMouseMovement(0.0f, 0.0f)
	{
	}

	QuakeCamera::~QuakeCamera()
	{
	}

	void QuakeCamera::AddToCameraPositionRelative(const glm::vec4& homoVectorToAdd, double deltaTime)
	{
		glm::vec3 rotatedVector = glm::vec3(mRotation * homoVectorToAdd);
		float speedMultiplier = 1.0f;
		if (input().key_down(key_code::left_shift)) {
			speedMultiplier = mFastMultiplier;
		}
		if (input().key_down(key_code::left_control)) {
			speedMultiplier = mSlowMultiplier;
		}
		Translate(mMoveSpeed * speedMultiplier * static_cast<float>(deltaTime) * rotatedVector);
		//log_verbose("cam-pos[%.2f, %.2f, %.2f]", GetPosition().x, GetPosition().y, GetPosition().z);
	}

	void QuakeCamera::AddToCameraPositionAbsolute(const glm::vec4& homoVectorToAdd, double deltaTime)
	{
		float speedMultiplier = 1.0f;
		if (input().key_down(key_code::left_shift)) {
			speedMultiplier = mFastMultiplier;
		}
		if (input().key_down(key_code::left_control)) {
			speedMultiplier = mSlowMultiplier;
		}
		Translate(mMoveSpeed * speedMultiplier * static_cast<float>(deltaTime) * homoVectorToAdd);
		//log_verbose("cam-pos[%.2f, %.2f, %.2f]", GetPosition().x, GetPosition().y, GetPosition().z);
	}

	void QuakeCamera::HandleInputOnly()
	{
		// switch mode
		if (input().key_pressed(key_code::tab)) {
			auto currentlyHidden = input().is_cursor_hidden();
			auto newMode = !currentlyHidden;
			mCaptureInput = newMode == true;
			input().set_cursor_hidden(newMode);
			input().center_cursor_position();
		}

		// display info
		if (input().key_pressed(key_code::i) 
			&& (input().key_down(key_code::left_control) || input().key_down(key_code::right_control))) {
			LOG_INFO(fmt::format("QuakeCamera's position: {}", to_string(GetPosition())));
			LOG_INFO(fmt::format("QuakeCamera's view-dir: {}", to_string(GetFrontVector())));
			LOG_INFO(fmt::format("QuakeCamera's up-vec:   {}", to_string(GetUpVector())));
			LOG_INFO(fmt::format("QuakeCamera's view-mat: {}", to_string(CalculateViewMatrix())));
		}
	}

	void QuakeCamera::initialize()
	{
		input().set_cursor_hidden(true);
	}

	void QuakeCamera::finalize()
	{
		input().set_cursor_hidden(false);
	}

	void QuakeCamera::update()
	{
		HandleInputOnly();
		if (!mCaptureInput) {
			return;
		}

		auto deltaCursor = input().delta_cursor_position();
		auto deltaTime = time().delta_time();

		// query the position of the mouse cursor
		auto mousePos = input().cursor_position();
		LOG_INFO(fmt::format("mousePos[{},{}]", mousePos.x, mousePos.y));

		// calculate how much the cursor has moved from the center of the screen
		auto mouseMoved = deltaCursor;
		LOG_INFO_EM(fmt::format("mouseMoved[{},{}]", mouseMoved.x, mouseMoved.y));

		// accumulate values and create rotation-matrix
		mAccumulatedMouseMovement.x += mRotationSpeed * static_cast<float>(mouseMoved.x);
		mAccumulatedMouseMovement.y += mRotationSpeed * static_cast<float>(mouseMoved.y);
		mAccumulatedMouseMovement.y = glm::clamp(mAccumulatedMouseMovement.y, -glm::half_pi<float>(), glm::half_pi<float>());
		glm::mat4 cameraRotation = glm::rotate(mAccumulatedMouseMovement.x, kUnitVec3Y) * glm::rotate(mAccumulatedMouseMovement.y, kUnitVec3X);

		// set the rotation
		set_rotation(cameraRotation);

		// move camera to new position
		if (input().key_down(key_code::w))
			AddToCameraPositionRelative(kFrontVec4, deltaTime);
		if (input().key_down(key_code::s))
			AddToCameraPositionRelative(-kFrontVec4, deltaTime);
		if (input().key_down(key_code::d))
			AddToCameraPositionRelative(kRightVec4, deltaTime);
		if (input().key_down(key_code::a))
			AddToCameraPositionRelative(-kRightVec4, deltaTime);
		if (input().key_down(key_code::q))
			AddToCameraPositionAbsolute(-kUpVec4, deltaTime);
		if (input().key_down(key_code::e))
			AddToCameraPositionAbsolute(kUpVec4, deltaTime);

		// reset the mouse-cursor to the center of the screen
		input().center_cursor_position();
	}
}
