#include "FlatLoop.h"

#include <SimpleLog/SimpleLog.hpp>

FlatLoop::FlatLoop(sgrottel::ISimpleLog& log)
	:m_log{ log }
{
}

std::vector<glm::uvec2> FlatLoop::IsSelfintersecting(std::vector<glm::vec2> l) const
{
	m_log.Detail("Checking 2d project of loop for self-intersection");

	m_log.Error("Not implemented");

	return {};
}
