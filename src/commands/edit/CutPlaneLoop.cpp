#include "CutPlaneLoop.h"

#include "utilities/LoopsFromEdges.h"

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
#include <unordered_map>
#include <unordered_set>
//#include <vector>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::edit;

namespace
{
	float EdgeDist(const data::HashableEdge& edge, const glm::vec3& pt, const std::vector<glm::vec3>& vertices)
	{
		const glm::vec3& v0 = vertices.at(edge.i0);
		const glm::vec3& v1 = vertices.at(edge.i1);

		const glm::vec3 ve = v1 - v0;
		const glm::vec3 r = pt - v0;

		const float edgeLen2 = glm::dot(ve, ve);
		const float a = glm::dot(r, ve) / edgeLen2;

		if (a < 0.0)
		{
			return glm::distance(v0, pt);
		}
		if (a > 1.0)
		{
			return glm::distance(v1, pt);
		}

		return glm::distance(v0 + r * a, pt);
	}
}

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
	// compute edges on plane, adding new vertices to 'm_mesh' (we'll clean up later)
	std::vector<uint32_t> tris;
	std::unordered_set<data::HashableEdge> edges;
	std::unordered_map<data::HashableEdge, uint32_t> newVert;
	for (uint32_t ti = 0; ti < static_cast<uint32_t>(m_mesh->triangles.size()); ++ti)
	{
		const auto& t = m_mesh->triangles.at(ti);
		const float d[3] = { dist.at(t[0]), dist.at(t[1]), dist.at(t[2]) };
		const bool hasNeg = d[0] < 0.0f || d[1] < 0.0f || d[2] < 0.0f;
		const bool hasNul = d[0] == 0.0f || d[1] == 0.0f || d[2] == 0.0f;
		const bool hasPos = d[0] > 0.0f || d[1] > 0.0f || d[2] > 0.0f;

		if (!hasNul && !(hasNeg && hasPos))
		{
			continue;
		}
		tris.push_back(ti);

		if (hasNeg && hasPos)
		{
			// tri is cut
			uint32_t v0 = std::numeric_limits<uint32_t>::max(), v1 = std::numeric_limits<uint32_t>::max();
			for (uint32_t i = 0; i < 3; ++i)
			{
				const uint32_t j = (i + 1) % 3;
				if ((d[i] < 0.0f && d[j] > 0.0f) || (d[i] > 0.0f && d[j] < 0.0f))
				{
					// edge is cut
					data::HashableEdge e{ t[i], t[j] };
					if (!newVert.contains(e))
					{
						glm::vec3 v = m_plane->CutInterpolate(e, dist, m_mesh->vertices);
						newVert.insert({ e, static_cast<uint32_t>(m_mesh->vertices.size()) });
						m_mesh->vertices.push_back(v);
					}
					uint32_t vi = newVert.at(e);
					if (v0 == std::numeric_limits<uint32_t>::max())
					{
						v0 = vi;
					}
					else
					{
						assert(v1 == std::numeric_limits<uint32_t>::max());
						v1 = vi;
					}

				}
				else if (d[i] == 0)
				{
					// vertex on plane
					if (v0 == std::numeric_limits<uint32_t>::max())
					{
						v0 = t[i];
					}
					else
					{
						assert(v1 == std::numeric_limits<uint32_t>::max());
						v1 = t[i];
					}
				}
			}

			assert(v1 != std::numeric_limits<uint32_t>::max());
			edges.insert(data::HashableEdge{ v0, v1 });
		}
		else
		{
			assert(hasNul);
			if (d[0] == 0)
			{
				if (d[1] == 0)
				{
					edges.insert(data::HashableEdge{ t[0], t[1] });
				}
				else if (d[2] == 0)
				{
					edges.insert(data::HashableEdge{ t[0], t[2] });
				}
			}
			else if (d[1] == 0 && d[2] == 0)
			{
				edges.insert(data::HashableEdge{ t[1], t[2] });
			}
			// all other cases mean only one vertex is on the plane, aka the loop is formed by neighboring triangles only.
		}
	}

	// The loop to cut!
	std::vector<uint32_t> loop;
	{
		// collect all loops on the plane
		std::shared_ptr<std::vector<std::shared_ptr<std::vector<uint32_t>>>> openLoops;
		utilities::LoopsFromEdges(edges, openLoops, Log());

		auto [projX, projY] = m_plane->Make2DCoordSys();
		float minX = 0.0f;
		std::unordered_map<uint32_t, glm::vec2> pt2d;
		for (auto loop : *openLoops)
		{
			for (uint32_t vi : *loop)
			{
				if (pt2d.contains(vi)) continue;
				const glm::vec3 v = m_mesh->vertices.at(vi) - m_plane->Plane();
				const float x = glm::dot(v, projX);
				pt2d.insert(std::make_pair(vi, glm::vec2(x, glm::dot(v, projY))));
				if (x < minX)
				{
					minX = x;
				}
			}
		}

		if (openLoops->size() < 1)
		{
			Log().Error("No open loops extracted");
			return false;
		}
		else if (openLoops->size() == 1)
		{
			std::swap(loop, *openLoops->front());
		}
		else
		{
			assert(openLoops->size() > 1);
			// indentify selected loop

			float minLoopDist = std::numeric_limits<float>::max();
			size_t minLoopIdx = 0;
			for (size_t loopI = 0; loopI < openLoops->size(); ++loopI)
			{
				const auto& loop = *openLoops->at(loopI);
				float loopDist = std::numeric_limits<float>::max();
				uint32_t pIdx = loop.back();
				for (uint32_t idx : loop)
				{
					data::HashableEdge e{pIdx, idx};
					pIdx = idx;
					const float dist = EdgeDist(e, m_point, m_mesh->vertices);
					if (dist < loopDist)
					{
						loopDist = dist;
					}
				}
				if (minLoopDist > loopDist)
				{
					minLoopDist = loopDist;
					minLoopIdx = loopI;
				}
			}

			std::swap(loop, *openLoops->at(minLoopIdx));
		}

		// only keep triangles in "tris" that are in the selected `loop`
		{
			std::unordered_set<uint32_t> loopVerts;
			loopVerts.reserve(loop.size());
			for (uint32_t vi : loop)
			{
				loopVerts.insert(vi);
			}

			std::vector<uint32_t> keepTris;
			for (uint32_t ti : tris)
			{
				const auto& t = m_mesh->triangles.at(ti);
				bool keep = false;
				for (uint32_t i = 0; i < 3; ++i)
				{
					if (loopVerts.contains(t[i]))
					{
						keep = true;
						break;
					}
					auto hei = newVert.find(t.HashableEdge(i));
					if (hei != newVert.end())
					{
						if (loopVerts.contains(hei->second))
						{
							keep = true;
							break;
						}
					}
				}
				if (keep)
				{
					keepTris.push_back(ti);
				}
			}
			std::swap(keepTris, tris);
		}
	}

	// duplicate the loop for the neg size
	std::vector<uint32_t> loopNeg;
	loopNeg.reserve(loop.size());
	m_mesh->vertices.reserve(m_mesh->vertices.size() + loop.size());
	for (uint32_t vi : loop)
	{
		loopNeg.push_back(static_cast<uint32_t>(m_mesh->vertices.size()));
		m_mesh->vertices.push_back(m_mesh->vertices.at(vi));
	}



	// TODO


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

	m_mesh->RemoveIsolatedVertices();

	return false;
}
