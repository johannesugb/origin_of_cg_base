#include "quake_camera.h"

namespace cgb
{
	QuakeCamera::QuakeCamera() :
		m_rotation_speed(0.001f),
		m_move_speed(4.5f), // 4.5 m/s
		m_fast_multiplier(6.0f), // 27 m/s
		m_slow_multiplier(0.2f), // 0.9 m/s
		m_accumulated_mouse_movement(0.0f, 0.0f)
	{
	}

	QuakeCamera::~QuakeCamera()
	{
	}

	void QuakeCamera::AddToCameraPositionRelative(const glm::vec4& homoVectorToAdd, double deltaTime)
	{
		glm::vec3 rotatedVector = glm::vec3(m_rotation * homoVectorToAdd);
		float speedMultiplier = 1.0f;
		if (input().key_pressed(key_code::left_shift)) {
			speedMultiplier = m_fast_multiplier;
		}
		if (input().key_pressed(key_code::left_control)) {
			speedMultiplier = m_slow_multiplier;
		}
		Translate(m_move_speed * speedMultiplier * static_cast<float>(deltaTime) * rotatedVector);
		//log_verbose("cam-pos[%.2f, %.2f, %.2f]", GetPosition().x, GetPosition().y, GetPosition().z);
	}

	void QuakeCamera::AddToCameraPositionAbsolute(const glm::vec4& homoVectorToAdd, double deltaTime)
	{
		float speedMultiplier = 1.0f;
		if (input().key_pressed(key_code::left_shift)) {
			speedMultiplier = m_fast_multiplier;
		}
		if (input().key_pressed(key_code::left_control)) {
			speedMultiplier = m_slow_multiplier;
		}
		Translate(m_move_speed * speedMultiplier * static_cast<float>(deltaTime) * homoVectorToAdd);
		//log_verbose("cam-pos[%.2f, %.2f, %.2f]", GetPosition().x, GetPosition().y, GetPosition().z);
	}

	void QuakeCamera::HandleInputOnly()
	{
		auto wnd = current_composition().window_in_focus();
		auto width = wnd->width();
		auto height = wnd->height();

		// switch mode
		if (input().key_pressed(key_code::tab)) {
			auto currentlyHidden = wnd->is_cursor_hidden();
			auto newMode = !currentlyHidden;
			wnd->set_cursor_hidden(newMode);
			m_capture_input = newMode == true;
			wnd->center_cursor_position();
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
		current_composition().window_in_focus()->set_cursor_hidden(true);
	}

	void QuakeCamera::finalize()
	{
		current_composition().window_in_focus()->set_cursor_hidden(false);
	}

	void QuakeCamera::update()
	{
		HandleInputOnly();

		if (!m_capture_input) {
			return;
		}

		auto wnd = current_composition().window_in_focus();
		auto width = wnd->width();
		auto height = wnd->height();
		auto deltaTime = time().delta_time();

		// query the position of the mouse cursor
		auto mousePos = wnd->cursor_position();

		// calculate how much the cursor has moved from the center of the screen
		glm::ivec2 mouseMoved = glm::ivec2(width / 2 - mousePos.x, height / 2 - mousePos.y);

		// accumulate values and create rotation-matrix
		m_accumulated_mouse_movement.x += m_rotation_speed * static_cast<float>(mouseMoved.x);
		m_accumulated_mouse_movement.y += m_rotation_speed * static_cast<float>(mouseMoved.y);
		m_accumulated_mouse_movement.y = glm::clamp(m_accumulated_mouse_movement.y, -glm::half_pi<float>(), glm::half_pi<float>());
		glm::mat4 cameraRotation = glm::rotate(m_accumulated_mouse_movement.x, kUnitVec3Y) * glm::rotate(m_accumulated_mouse_movement.y, kUnitVec3X);

		// set the rotation
		set_rotation(cameraRotation);

		// move camera to new position
		if (input().key_pressed(key_code::w))
			AddToCameraPositionRelative(kFrontVec4, deltaTime);
		if (input().key_pressed(key_code::s))
			AddToCameraPositionRelative(-kFrontVec4, deltaTime);
		if (input().key_pressed(key_code::d))
			AddToCameraPositionRelative(kSideVec4, deltaTime);
		if (input().key_pressed(key_code::a))
			AddToCameraPositionRelative(-kSideVec4, deltaTime);
		if (input().key_pressed(key_code::q))
			AddToCameraPositionAbsolute(-kUpVec4, deltaTime);
		if (input().key_pressed(key_code::e))
			AddToCameraPositionAbsolute(kUpVec4, deltaTime);

		// reset the mouse-cursor to the center of the screen
		wnd->center_cursor_position();
	}
}
