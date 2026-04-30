#include "CutPlaneLoop.h"

//#include "utilities/LoopsFromEdges.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

//#pragma warning(push)
//#pragma warning(disable : 4702)
//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <CGAL/Constrained_Delaunay_triangulation_2.h>
//#include <CGAL/Triangulation_vertex_base_with_info_2.h>
//#include <CGAL/Constrained_triangulation_face_base_2.h>
//#include <CGAL/Triangulation_data_structure_2.h>
//#pragma warning(pop)
//
//#include <algorithm>
//#include <functional>
//#include <unordered_map>
//#include <unordered_set>
//#include <vector>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::edit;

CutPlaneLoop::CutPlaneLoop(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
	, m_mesh{ nullptr }
	, m_plane{ std::make_shared<data::HalfSpace>() }
	, m_point{ 0.0f, 0.0f, 0.0f }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::HalfSpace>("Plane", m_plane);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("Point", m_point);
}

bool CutPlaneLoop::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh not set");
		return false;
	}
	if (!m_plane)
	{
		Log().Error("Plane not set");
		return false;
	}
	if (std::abs(m_plane->Dist(m_point)) > 0.00001f)
	{
		Log().Warning("Point not in plane");
	}


	std::vector<float> dist(m_mesh->vertices.size());
	std::transform(
		m_mesh->vertices.begin(),
		m_mesh->vertices.end(),
		dist.begin(),
		std::bind(static_cast<float (data::HalfSpace::*)(glm::vec3 const&) const>(&data::HalfSpace::Dist), m_plane, std::placeholders::_1)
	);

	// first select all triangles touching the plane
	std::vector<uint32_t> tris;
	for (uint32_t ti = 0; ti < static_cast<uint32_t>(m_mesh->triangles.size()); ++ti)
	{
		const float d0 = dist.at(m_mesh->triangles.at(ti)[0]);
		const float d1 = dist.at(m_mesh->triangles.at(ti)[1]);
		const float d2 = dist.at(m_mesh->triangles.at(ti)[2]);

		const bool hasNeg = d0 < 0.0f || d1 < 0.0f || d2 < 0.0f;
		const bool hasNul = d0 == 0.0f || d1 == 0.0f || d2 == 0.0f;
		const bool hasPos = d0 > 0.0f || d1 > 0.0f || d2 > 0.0f;

		if (hasNul || (hasNeg && hasPos))
		{
			tris.push_back(ti);
		}
	}

	std::erase_if(m_mesh->triangles,
		[&](const data::Triangle& t)
		{
			for (uint32_t ti : tris)
			{
				if (
					m_mesh->triangles.at(ti)[0] == t[0]
					&& m_mesh->triangles.at(ti)[1] == t[1]
					&& m_mesh->triangles.at(ti)[2] == t[2]
					)
				{
					return true;
				}
			}
			return false;
		});


	return false;
}
