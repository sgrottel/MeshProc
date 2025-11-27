#include "OpenBorder.h"

#include "algo/LoopsFromEdges.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#include <unordered_map>

#include <iostream>

using namespace meshproc;

OpenBorder::OpenBorder(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::Out, ParamType::MultiIndices>("EdgeLists", m_edgeLists);
}

bool OpenBorder::Invoke()
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

	algo::LoopsFromEdges(openEdges, m_edgeLists, Log());
	Log().Detail("Found %d open border loops", static_cast<int>(m_edgeLists->size()));

	return true;
}
