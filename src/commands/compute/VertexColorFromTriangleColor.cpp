#include "VertexColorFromTriangleColor.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;

compute::VertexColorFromTriangleColor::VertexColorFromTriangleColor(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::FloatList>("TriangleColors", m_tricol);
	AddParamBinding<ParamMode::Out, ParamType::FloatList>("VertexColors", m_vertcol);
}

bool compute::VertexColorFromTriangleColor::Invoke()
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
	if (!m_tricol)
	{
		Log().Error("TriangleColors is empty");
		return false;
	}
	if (m_tricol->size() != m_mesh->triangles.size())
	{
		Log().Error("TriangleColors is of different size than Mesh number of triangles");
		return false;
	}

	m_vertcol = std::make_shared<std::vector<float>>(m_mesh->vertices.size(), 0.0f);
	std::vector<uint32_t> cnt(m_mesh->vertices.size(), 0u);

	for (size_t i = 0; i < m_mesh->triangles.size(); ++i)
	{
		const data::Triangle& t = m_mesh->triangles[i];
		for (uint32_t j = 0; j < 3; ++j)
		{
			const uint32_t vi = t[j];
			m_vertcol->at(vi) += m_tricol->at(i);
			cnt.at(vi)++;
		}
	}

	for (size_t i = 0; i < cnt.size(); ++i)
	{
		const uint32_t c = cnt[i];
		if (c == 0) continue;
		const float f = 1.0f / static_cast<float>(c);
		m_vertcol->at(i) *= f;
	}

	return true;
}
