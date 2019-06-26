#pragma once
#include "cp_interpolation.h"

namespace cgb
{
	class catmull_rom_spline : public cp_interpolation
	{
	public:
		catmull_rom_spline() = default;
		// Initializes a instance with a set of control points
		catmull_rom_spline(std::vector<glm::vec3> pControlPoints);
		catmull_rom_spline(catmull_rom_spline&&) = default;
		catmull_rom_spline(const catmull_rom_spline&) = default;
		catmull_rom_spline& operator=(catmull_rom_spline&&) = default;
		catmull_rom_spline& operator=(const catmull_rom_spline&) = default;
		~catmull_rom_spline() = default;

		virtual glm::vec3 value_at(float t) override;

		virtual glm::vec3 slope_at(float t) override;

		virtual float arc_length() override;
	};
}
