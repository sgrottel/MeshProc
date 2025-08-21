#include "SelectTrianglesFromSelectedVertices.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_set>

using namespace meshproc;

SelectTrianglesFromSelectedVertices::SelectTrianglesFromSelectedVertices(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Indices>("SelectedVertices", m_vertSel);
	AddParamBinding<ParamMode::In, ParamType::UInt32>("NumReqSelVert", m_reqVertSel);
	AddParamBinding<ParamMode::Out, ParamType::Indices>("SelectedTriangles", m_trisSel);
}

bool SelectTrianglesFromSelectedVertices::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_vertSel)
	{
		Log().Error("SelectedVertices is empty");
		return false;
	}
	if (m_reqVertSel < 1)
	{
		Log().Warning("NumReqSelVert < 1 means all triangles will be selected");
	}
	if (m_reqVertSel > 3)
	{
		Log().Warning("NumReqSelVert > 3 means no triangles will be selected");
		return false;
	}

	std::unordered_set<uint32_t> selV;
	selV.reserve(m_vertSel->size());
	for (uint32_t v : *m_vertSel)
	{
		selV.insert(v);
	}

	m_trisSel = std::make_shared<std::vector<uint32_t>>();

	for (size_t i = 0; i < m_mesh->triangles.size(); ++i)
	{
		auto const& t = m_mesh->triangles.at(i);
		uint32_t sc = 0;
		for (size_t j = 0; j < 3; ++j)
		{
			if (selV.contains(t[j])) sc++;
		}
		if (sc >= m_reqVertSel)
		{
			m_trisSel->push_back(static_cast<uint32_t>(i));
		}
	}

	Log().Detail("Selected %d / %d triangles", static_cast<int>(m_trisSel->size()), static_cast<int>(m_mesh->triangles.size()));

	return true;
}
