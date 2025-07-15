#include "LinearExtrude.h"

#include <SimpleLog/SimpleLog.hpp>

#include <algorithm>
#include <numeric>

using namespace meshproc;

LinearExtrude::LinearExtrude(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::InOut, ParamType::VertexSelection>("Loop", m_loop);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("Dir", m_dir);
}

bool LinearExtrude::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh not set");
		return false;
	}
	if (!m_loop)
	{
		Log().Error("Loop not set");
		return false;
	}

	const size_t voff = m_mesh->vertices.size();
	const size_t toff = m_mesh->triangles.size();
	const size_t lsize = m_loop->size();
	m_mesh->vertices.reserve(voff + lsize);
	m_mesh->triangles.reserve(m_mesh->triangles.size() + lsize * 2);

	for (size_t i = 0; i < lsize; ++i)
	{
		m_mesh->vertices.push_back(m_mesh->vertices.at(m_loop->at(i)) + m_dir);
		m_mesh->AddQuad(
			m_loop->at(i),
			m_loop->at((i + 1) % lsize),
			static_cast<uint32_t>(voff + i),
			static_cast<uint32_t>(voff + ((i + 1) % lsize)));
	}

	// probe with the first of the new triangles
	data::Triangle& ntp = m_mesh->triangles.at(toff);
	bool needToFlip = false;
	for (size_t i = 0; i < toff; ++i)
	{
		data::Triangle const& t = m_mesh->triangles.at(i);
		glm::uvec2 ce = t.CommonEdge(ntp);
		if (ce.x == 0 && ce.y == 0) continue;
		needToFlip = !t.OrientationMatches(m_mesh->vertices, ntp, ce);
		break;
	}
	if (needToFlip)
	{
		for (size_t i = toff; i < m_mesh->triangles.size(); ++i)
		{
			m_mesh->triangles.at(i).Flip();
		}
	}

	for (size_t i = 0; i < lsize; ++i)
	{
		m_loop->at(i) = static_cast<uint32_t>(voff + i);
	}

	return true;
}
