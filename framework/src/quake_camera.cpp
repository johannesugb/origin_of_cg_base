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

	void QuakeCamera::AddToCameraPosition(const glm::vec3& translation, double deltaTime)
	{
		float speedMultiplier = 1.0f;
		if (input().key_down(key_code::left_shift)) {
			speedMultiplier = mFastMultiplier;
		}
		if (input().key_down(key_code::left_control)) {
			speedMultiplier = mSlowMultiplier;
		}
		translate(*this, mMoveSpeed * speedMultiplier * static_cast<float>(deltaTime) * translation);
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
			LOG_INFO(fmt::format("QuakeCamera's position: {}", to_string(translation())));
			LOG_INFO(fmt::format("QuakeCamera's view-dir: {}", to_string(front(*this))));
			LOG_INFO(fmt::format("QuakeCamera's up-vec:   {}", to_string(up(*this))));
			LOG_INFO(fmt::format("QuakeCamera's position and orientation:\n{}", to_string(mMatrix)));
			LOG_INFO(fmt::format("QuakeCamera's view-mat:\n{}", to_string(CalculateViewMatrix())));
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
		//LOG_INFO(fmt::format("mousePos[{},{}]", mousePos.x, mousePos.y));

		// calculate how much the cursor has moved from the center of the screen
		auto mouseMoved = deltaCursor;
		//LOG_INFO_EM(fmt::format("mouseMoved[{},{}]", mouseMoved.x, mouseMoved.y));

		// accumulate values and create rotation-matrix
		glm::quat rotHoriz = glm::quat_cast(glm::rotate(mRotationSpeed * static_cast<float>(mouseMoved.x), glm::vec3(0.f, 1.f, 0.f)));
		glm::quat rotVert =  glm::quat_cast(glm::rotate(mRotationSpeed * static_cast<float>(mouseMoved.y), glm::vec3(1.f, 0.f, 0.f)));
		set_rotation(rotHoriz * rotation() * rotVert);

		// move camera to new position
		if (input().key_down(key_code::w))
			AddToCameraPosition(front(*this), deltaTime);
		if (input().key_down(key_code::s))
			AddToCameraPosition(back(*this), deltaTime);
		if (input().key_down(key_code::d))
			AddToCameraPosition(right(*this), deltaTime);
		if (input().key_down(key_code::a))
			AddToCameraPosition(left(*this), deltaTime);
		if (input().key_down(key_code::e))
			AddToCameraPosition(up(*this), deltaTime);
		if (input().key_down(key_code::q))
			AddToCameraPosition(down(*this), deltaTime);

		// reset the mouse-cursor to the center of the screen
		input().center_cursor_position();
	}
}
