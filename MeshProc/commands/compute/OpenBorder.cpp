#include "OpenBorder.h"

#include "utilities/LoopsFromEdges.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;

compute::OpenBorder::OpenBorder(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::Out, ParamType::IndexListList>("EdgeLists", m_edgeLists);
}

bool compute::OpenBorder::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (m_mesh->triangles.empty())
	{
		Log().Error("Mesh has no triangles");
		return false;
	}

	Log().Detail("Detecting open border edges");

	std::unordered_set<data::HashableEdge> openEdges = m_mesh->CollectOpenEdges();

	utilities::LoopsFromEdges(openEdges, m_edgeLists, Log());
	Log().Detail("Found %d open border loops", static_cast<int>(m_edgeLists->size()));

	return true;
}
