#include "HalfSpace.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc::data;

std::tuple<glm::vec3, glm::vec3> HalfSpace::Make2DCoordSys() const
{
	const glm::vec3 projXTemp = (std::abs(std::abs(Normal().x) - 1.0f) < 0.00001f)
		? glm::vec3{ 0.0f, 1.0f, 0.0f }
	: glm::vec3{ 1.0f, 0.0f, 0.0f };
	const glm::vec3 projY = glm::normalize(glm::cross(Normal(), projXTemp));
	const glm::vec3 projX = glm::normalize(glm::cross(projY, Normal()));
	return std::make_tuple(projX, projY);
}
