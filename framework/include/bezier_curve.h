#pragma once
#include "cp_interpolation.h"

namespace cgb
{
	class bezier_curve : public cp_interpolation
	{
	public:
		bezier_curve() = default;
		// Initializes a instance with a set of control points
		bezier_curve(std::vector<glm::vec3> pControlPoints);
		bezier_curve(bezier_curve&&) = default;
		bezier_curve(const bezier_curve&) = default;
		bezier_curve& operator=(bezier_curve&&) = default;
		bezier_curve& operator=(const bezier_curve&) = default;
		~bezier_curve() = default;

		virtual glm::vec3 value_at(float t) override;

		virtual glm::vec3 slope_at(float t) override;
	};
}
