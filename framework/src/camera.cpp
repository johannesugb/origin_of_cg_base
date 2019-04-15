#include "camera.h"

namespace cgb
{
	Camera::Camera()
		: m_projection_matrix(1.0f),
		m_projection_config(CameraProjectionConfig_None),
		m_near_plane(0.0f),
		m_far_plane(0.0f),
		m_fov(0.0f),
		m_left(0.0f),
		m_right(0.0f),
		m_top(0.0f),
		m_bottom(0.0f)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::set_projection_matrix(glm::mat4 matrix)
	{
		m_projection_config = CameraProjectionConfig_None;
		m_projection_matrix = std::move(matrix);
	}

	const glm::mat4& Camera::projection_matrix() const
	{
		return m_projection_matrix;
	}

	glm::mat4 Camera::CalculateViewMatrix()
	{
		glm::mat4 vM = glm::mat4(
			 mMatrix[0],
			-mMatrix[1],
			-mMatrix[2],
			 mMatrix[3]
		);
		vM = mMatrix;
		return glm::inverse(vM);
	}

	void Camera::CopyFrom(const Camera& other)
	{
		m_projection_matrix = other.m_projection_matrix;
		m_projection_config = other.m_projection_config;
		m_near_plane = other.m_near_plane;
		m_far_plane = other.m_far_plane;
		m_fov = other.m_fov;
		m_aspect = other.m_aspect;
		m_left = other.m_left;
		m_right = other.m_right;
		m_top = other.m_top;
		m_bottom = other.m_bottom;
		UpdateProjectionMatrix();
	}

	void Camera::SetPerspectiveProjection(float fov, float aspect, float nearPlane, float farPlane)
	{
		m_projection_config = CameraProjectionConfig_Perspective;
		m_fov = fov;
		m_aspect = aspect;
		m_near_plane = nearPlane;
		m_far_plane = farPlane;
		UpdateProjectionMatrix();
	}

	void Camera::SetOrthogonalProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane)
	{
		m_projection_config = CameraProjectionConfig_Orthogonal;
		m_left = left;
		m_right = right;
		m_top = top;
		m_bottom = bottom;
		m_near_plane = nearPlane;
		m_far_plane = farPlane;
		UpdateProjectionMatrix();
	}

	CameraProjectionConfig Camera::projection_config() const
	{
		return m_projection_config;
	}

	float Camera::near_plane() const
	{
		return m_near_plane;
	}

	float Camera::far_plane() const
	{
		return m_far_plane;
	}

	float Camera::field_of_view() const
	{
		return m_fov;
	}

	float Camera::aspect_ratio() const
	{
		return m_aspect;
	}

	void Camera::set_near_plane(float value)
	{
		m_near_plane = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_far_plane(float value)
	{
		m_far_plane = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_field_of_view(float value)
	{
		m_fov = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_aspect_ratio(float value)
	{
		m_aspect = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_left(float value)
	{
		m_left = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_right(float value)
	{
		m_right = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_top(float value)
	{
		m_top = value;
		UpdateProjectionMatrix();
	}

	void Camera::set_bottom(float value)
	{
		m_bottom = value;
		UpdateProjectionMatrix();
	}

	void Camera::UpdateProjectionMatrix()
	{
		switch (m_projection_config) {
		case CameraProjectionConfig_None:
			break;
		case CameraProjectionConfig_Perspective:
			{
				m_projection_matrix = glm::perspective(m_fov, m_aspect, m_near_plane, m_far_plane);

				auto tanHalfFovy = glm::tan(m_fov / 2.f);

				glm::mat4 m(0.0f);
				m[0][0] = 1.f / m_aspect * tanHalfFovy;
				m[1][1] = 1.f / tanHalfFovy;
				m[2][2] = m_far_plane / (m_near_plane - m_far_plane);
				m[2][3] = -1.f;
				m[3][2] = -(m_far_plane * m_near_plane) / (m_far_plane - m_near_plane);

				m_projection_matrix = m;

				break;
			}
		case CameraProjectionConfig_Orthogonal:
			{


				glm::mat4 m(1.0f);
				auto R_L = m_right - m_left;
				auto T_B = m_top - m_bottom;
				auto F_N = m_far_plane - m_near_plane;

				m[0][0] = 2.f / R_L;
				m[1][1] = 2.f / T_B;
				m[2][2] = 1.f / F_N;
				m[3][0] = (m_right + m_left) / R_L;
				m[3][1] = (m_top + m_bottom) / T_B;
				m[3][2] = (m_near_plane + m_far_plane) / F_N;
			
				m_projection_matrix = m;
				//m_projection_matrix = glm::ortho(m_left, m_right, m_bottom, m_top, m_near_plane, m_far_plane);

				break;
			}
		}
	}

	float Camera::CalcZBufferDepth(const glm::vec3& posWS)
	{
		auto posSS = projection_matrix() * CalculateViewMatrix() * glm::vec4(posWS, 1.0f);
		float depth = posSS.z / posSS.w;
		return depth * 0.5f + 0.5f;
	}

	float Camera::CalcZBufferDepth(transform* transform)
	{
		return CalcZBufferDepth(transform->translation());
	}
}
