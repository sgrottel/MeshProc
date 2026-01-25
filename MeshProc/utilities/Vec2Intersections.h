#pragma once

#include <glm/glm.hpp>

namespace meshproc
{
	namespace utilities
	{
		namespace vec2segments
		{
			bool DoesIntersect(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& d);
			glm::vec2 CalcIntersection(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& d);
		}
	}
}
