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
