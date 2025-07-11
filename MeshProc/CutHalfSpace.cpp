#include "CutHalfSpace.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#include <algorithm>
#include <functional>
//#include <array>
//#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace meshproc;

CutHalfSpace::CutHalfSpace(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
	, m_mesh{ nullptr }
	, m_halfSpace{}
	, m_openLoops{ nullptr }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("PlaneNormal", m_halfSpace.GetPlaneNormalParam());
	AddParamBinding<ParamMode::In, ParamType::Float>("PlaneDist", m_halfSpace.GetPlaneDistParam());
	AddParamBinding<ParamMode::Out, ParamType::MultiVertexSelection>("OpenLoops", m_openLoops);
}

bool CutHalfSpace::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh not set");
		return false;
	}
	if (!m_halfSpace.ValidateParams(Log()))
	{
		return false;
	}

	std::vector<float> dist(m_mesh->vertices.size());
	std::transform(
		m_mesh->vertices.begin(),
		m_mesh->vertices.end(),
		dist.begin(),
		std::bind(static_cast<float (data::HalfSpace::*)(glm::vec3 const&) const>(&data::HalfSpace::Dist), m_halfSpace, std::placeholders::_1)
	);

	// first: remove all triangles fully placed in negative half space
	std::erase_if(
		m_mesh->triangles,
		[&dist](data::Triangle const& t)
		{
			return (dist[t[0]] <= 0.0f)
				&& (dist[t[1]] <= 0.0f)
				&& (dist[t[2]] <= 0.0f);
		});

	// then: move the triangles cut by the half space plane into a separate container
	std::vector<data::Triangle> border;
	auto it = std::partition(
		m_mesh->triangles.begin(),
		m_mesh->triangles.end(),
		[&dist](data::Triangle const& t)
		{
			return (dist[t[0]] >= 0.0f)
				&& (dist[t[1]] >= 0.0f)
				&& (dist[t[2]] >= 0.0f);
		});
	border.insert(border.end(),
		std::make_move_iterator(it),
		std::make_move_iterator(m_mesh->triangles.end()));
	m_mesh->triangles.erase(it, m_mesh->triangles.end());

	// cut border (hashable)edges and generate new triangles and vertices
	const size_t oldTriCount = m_mesh->triangles.size();
	std::unordered_map<data::HashableEdge, uint32_t> newVert;
	std::vector<uint32_t> triVerts;
	triVerts.reserve(6);
	for (data::Triangle const& t : border)
	{
		triVerts.clear();

		for (size_t i = 0; i < 3; ++i)
		{
			if (dist[t[i]] >= 0.0f)
			{
				triVerts.push_back(t[i]);
			}
		}
		for (size_t i = 0; i < 3; ++i)
		{
			const auto he = t.HashableEdge(static_cast<uint32_t>(i));
			if (m_halfSpace.IsCut(he, dist))
			{
				if (newVert.contains(he))
				{
					triVerts.push_back(newVert.at(he));
				}
				else
				{
					glm::vec3 nv = m_halfSpace.CutInterpolate(he, dist, m_mesh->vertices);
					uint32_t nvIdx = static_cast<uint32_t>(m_mesh->vertices.size());
					m_mesh->vertices.push_back(nv);
					newVert.insert(std::make_pair(he, nvIdx));
					triVerts.push_back(nvIdx);
				}
				if (triVerts.size() == 4)
				{
					if (he.Has(triVerts[0]))
					{
						std::swap(triVerts[2], triVerts[3]);
					}
				}
			}
		}

		if (triVerts.size() == 3)
		{
			m_mesh->triangles.push_back(data::Triangle(triVerts[0], triVerts[1], triVerts[2]));
			// memorize original triangle
		}
		else if (triVerts.size() == 4)
		{
			m_mesh->AddQuad(triVerts[0], triVerts[2], triVerts[1], triVerts[3]);
			// memorize original triangle
		}
		else
		{
			Log().Warning("Triangle unexpectedly cut into %d pieces", static_cast<int>(triVerts.size()));
		}
	}
	// TODO: Implement triangle flip by comparing new normal with normal of original triangle

	// remove all unused vertices
	// TODO: Implement

	// finally collect open edges
	// TODO: Implement - cf OpenBorder, but maybe synergies from vertex renumberation above

	return true;
}
