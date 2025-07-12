#include "HalfSpace.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc::data;

bool HalfSpace::ValidateParams(sgrottel::ISimpleLog const& log)
{
	if (glm::length(m_planeNormalParam) < 0.00001f)
	{
		log.Error("PlaneNormal too small (zero?)");
		return false;
	}

	m_normal = glm::normalize(m_planeNormalParam);
	m_plane = m_normal * m_planeDistParam;

	return true;
}

std::tuple<glm::vec3, glm::vec3> HalfSpace::Make2DCoordSys() const
{
	const glm::vec3 projXTemp = (std::abs(std::abs(Normal().x) - 1.0f) < 0.00001f)
		? glm::vec3{ 0.0f, 1.0f, 0.0f }
	: glm::vec3{ 1.0f, 0.0f, 0.0f };
	const glm::vec3 projY = glm::normalize(glm::cross(Normal(), projXTemp));
	const glm::vec3 projX = glm::normalize(glm::cross(projY, Normal()));
	return std::make_tuple(projX, projY);
}
