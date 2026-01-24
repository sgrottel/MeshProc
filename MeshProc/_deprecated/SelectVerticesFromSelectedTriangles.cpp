#include "SelectVerticesFromSelectedTriangles.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_set>

using namespace meshproc;

SelectVerticesFromSelectedTriangles::SelectVerticesFromSelectedTriangles(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Indices>("SelectedTriangles", m_trisSel);
	AddParamBinding<ParamMode::Out, ParamType::Indices>("SelectedVertices", m_vertSel);
}

bool SelectVerticesFromSelectedTriangles::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_trisSel)
	{
		Log().Error("SelectedTriangles is empty");
		return false;
	}

	std::unordered_set<uint32_t> selV;
	for (uint32_t tidx : *m_trisSel)
	{
		auto const& t = m_mesh->triangles.at(tidx);
		for (size_t j = 0; j < 3; ++j)
		{
			selV.insert(t[j]);
		}
	}

	m_vertSel = std::make_shared<std::vector<uint32_t>>();
	m_vertSel->resize(selV.size());
	std::copy(selV.begin(), selV.end(), m_vertSel->begin());

	Log().Detail("Selected %d / %d vertices", static_cast<int>(selV.size()), static_cast<int>(m_mesh->vertices.size()));

	return true;
}
