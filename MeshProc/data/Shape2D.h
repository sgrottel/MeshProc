#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

namespace meshproc
{
	namespace data
	{
		class Shape2D
		{
		public:
			std::unordered_map<size_t, std::vector<glm::vec2>> loops;
		};
	}
}
