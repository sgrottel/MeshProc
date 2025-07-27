#include "Subdivision.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_map>

using namespace meshproc;

Subdivision::Subdivision(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::Out, ParamType::VertexSelection>("NewVertices", m_newVertices);
}

bool Subdivision::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}

	std::unordered_map<data::HashableEdge, uint32_t> edgeSubDivs;
	for (auto const& t : m_mesh->triangles)
	{
		for (uint32_t i = 0; i < 3; ++i)
		{
			const auto he = t.HashableEdge(i);
			if (!edgeSubDivs.contains(he))
			{
				edgeSubDivs.insert(std::make_pair(he, static_cast<uint32_t>(m_mesh->vertices.size() + edgeSubDivs.size())));
			}
		}
	}
	m_newVertices = std::make_shared<std::vector<uint32_t>>();
	m_newVertices->reserve(edgeSubDivs.size());
	m_mesh->vertices.resize(m_mesh->vertices.size() + edgeSubDivs.size());
	for (auto const& e : edgeSubDivs)
	{
		m_newVertices->push_back(e.second);
		m_mesh->vertices.at(e.second) = (m_mesh->vertices.at(e.first.i0) + m_mesh->vertices.at(e.first.i1)) * 0.5f;
	}

	const size_t inTriCnt = m_mesh->triangles.size();
	m_mesh->triangles.resize(inTriCnt * 4);
	for (size_t ti = 0; ti < inTriCnt; ++ti)
	{
		auto& t = m_mesh->triangles.at(ti);
		//const uint32_t i0 = t[0];
		const uint32_t i1 = t[1];
		const uint32_t i2 = t[2];
		const uint32_t i0_1 = edgeSubDivs.at(t.HashableEdge(0));
		const uint32_t i1_2 = edgeSubDivs.at(t.HashableEdge(1));
		const uint32_t i2_0 = edgeSubDivs.at(t.HashableEdge(2));

		//t[0] = i0;
		t[1] = i0_1;
		t[2] = i2_0;

		m_mesh->triangles.at(inTriCnt + ti * 3) = { i0_1, i1_2, i2_0 };
		m_mesh->triangles.at(inTriCnt + ti * 3 + 1) = { i0_1, i1, i1_2 };
		m_mesh->triangles.at(inTriCnt + ti * 3 + 2) = { i1_2, i2, i2_0 };
	}

	return true;
}