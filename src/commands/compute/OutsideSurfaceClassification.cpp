#include "OutsideSurfaceClassification.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;

compute::OutsideSurfaceClassification::OutsideSurfaceClassification(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::Out, ParamType::FloatList>("FaceType", m_facetype);
}

bool compute::OutsideSurfaceClassification::Invoke()
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

	m_facetype = std::make_shared<std::vector<float>>(m_mesh->triangles.size(), 0.0f);

	// for each triangle determin how much outside can be seen, cf. ambient occlusion

	//Log().Detail("Detecting open border edges");

	//std::unordered_set<data::HashableEdge> openEdges = m_mesh->CollectOpenEdges();

	//utilities::LoopsFromEdges(openEdges, m_edgeLists, Log());
	//Log().Detail("Found %d open border loops", static_cast<int>(m_edgeLists->size()));

	return false;
}
