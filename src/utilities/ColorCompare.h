#pragma once

#include <glm/glm.hpp>

namespace meshproc
{
	class ColorCompare
	{
	public:
		static constexpr const float Epsilon = 0.005f;

		static inline const float ManhattenDistance(const glm::vec3& a, const glm::vec3& b) noexcept
		{
			return std::max(std::max(
				std::abs(a.x - b.x),
				std::abs(a.y - b.y)),
				std::abs(a.z - b.z));
		}

		static inline const bool IsNearlyEqual(const glm::vec3& a, const glm::vec3& b) noexcept
		{
			return ManhattenDistance(a, b) < Epsilon;
		}
	};
}
