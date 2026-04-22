#include "CloseLoopWithPin.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;

edit::CloseLoopWithPin::CloseLoopWithPin(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::IndexList>("Loop", m_loop);
	AddParamBinding<ParamMode::Out, ParamType::UInt32>("NewVertexIndex", m_newVertexIndex);
}

bool edit::CloseLoopWithPin::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_loop)
	{
		Log().Error("Loop is empty");
		return false;
	}

	Log().Detail("Closing loop by adding a single pin vertex in the center");

	glm::vec3 center{ 0.0 };
	for (uint32_t i : *m_loop)
	{
		center += m_mesh->vertices[i];
	}
	center /= static_cast<float>(m_loop->size());

	uint32_t vi1 = m_loop->at(0);
	uint32_t vi2 = m_loop->at(1);
	data::Triangle oldTri;
	for (const auto& t : m_mesh->triangles)
	{
		if (!t.HasIndex(vi1)) continue;
		if (!t.HasIndex(vi2)) continue;
		oldTri = t;
		break;
	}

	m_newVertexIndex = static_cast<uint32_t>(m_mesh->vertices.size());
	m_mesh->vertices.push_back(center);

	data::Triangle newTri{ vi1, vi2, m_newVertexIndex };
	bool flip = !newTri.OrientationMatches(m_mesh->vertices, oldTri, { vi1, vi2 });

	m_mesh->triangles.reserve(m_mesh->triangles.size() + m_loop->size());
	for (size_t i = 0; i < m_loop->size(); ++i)
	{
		size_t i1 = i;
		size_t i2 = (i + 1) % m_loop->size();
		if (flip)
		{
			std::swap(i1, i2);
		}
		m_mesh->triangles.push_back({ m_loop->at(i1), m_loop->at(i2), m_newVertexIndex });
	}

	m_newVertexIndex++; // as lua param this must be 1-based

	return true;
}
