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

		float rotation_speed() const { return m_rotation_speed; }
		float move_speed() const { return m_move_speed; }
		float fast_multiplier() const { return m_fast_multiplier; }
		float slow_multiplier() const { return m_slow_multiplier; }

		void set_rotation_speed(float value) { m_rotation_speed = value; }
		void set_move_speed(float value) { m_move_speed = value; }
		void set_fast_multiplier(float value) { m_fast_multiplier = value; }
		void set_slow_multiplier(float value) { m_slow_multiplier = value; }

	private:
		float m_rotation_speed;
		float m_move_speed;
		float m_fast_multiplier;
		float m_slow_multiplier;

		glm::vec2 m_accumulated_mouse_movement;
		bool m_capture_input;

		std::function<void(GLFWwindow*, int, int, int, int)> m_key_handler;

		void AddToCameraPositionRelative(const glm::vec4& homoVectorToAdd, double deltaTime);
		void AddToCameraPositionAbsolute(const glm::vec4& homoVectorToAdd, double deltaTime);

	};
}
