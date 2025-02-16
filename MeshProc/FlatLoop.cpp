#include "FlatLoop.h"

#include <SimpleLog/SimpleLog.hpp>

FlatLoop::FlatLoop(sgrottel::ISimpleLog& log)
	:m_log{ log }
{
}

std::vector<glm::uvec2> FlatLoop::IsSelfintersecting(std::vector<glm::vec2> l) const
{
	m_log.Detail("Checking 2d project of loop for self-intersection");

	// TODO: detect loop self-intersection in projected 2d space
	// if intersecting -> resolve by collapsing both offending edges,
	//                    remove degenerated triangles,
	//                    flip triangles if necessary (should be on one side).
	//                    repeat until no longer self-intersecting.
	//                    then break loops.

	m_log.Error("Not implemented");

	return {};
}
