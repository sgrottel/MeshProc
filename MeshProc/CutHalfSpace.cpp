#include "CutHalfSpace.h"

#include "algo/LoopsFromEdges.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Constrained_triangulation_face_base_2.h>
#include <CGAL/Triangulation_data_structure_2.h>

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <vector>

using namespace meshproc;

namespace
{

	// Returns +1 if counter-clockwise, -1 if clockwise, 0 if colinear
	int orientation(const glm::vec2& p, const glm::vec2& q, const glm::vec2& r) {
		double val = (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
		if (val > 0) return 1;     // left turn
		if (val < 0) return -1;    // right turn
		return 0;                  // colinear
	}

	// Checks if r lies on segment pq
	bool on_segment(const glm::vec2& p, const glm::vec2& q, const glm::vec2& r) {
		return (std::min)(p.x, q.x) <= r.x && r.x <= (std::max)(p.x, q.x)
			&& (std::min)(p.y, q.y) <= r.y && r.y <= (std::max)(p.y, q.y);
	}

	// Main intersection test
	bool segments_intersect(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& d) {
		int o1 = orientation(a, b, c);
		int o2 = orientation(a, b, d);
		int o3 = orientation(c, d, a);
		int o4 = orientation(c, d, b);

		// General case
		if (o1 != o2 && o3 != o4) return true;

		// Colinear special cases
		if (o1 == 0 && on_segment(a, b, c)) return true;
		if (o2 == 0 && on_segment(a, b, d)) return true;
		if (o3 == 0 && on_segment(c, d, a)) return true;
		if (o4 == 0 && on_segment(c, d, b)) return true;

		return false;
	}

}

CutHalfSpace::CutHalfSpace(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
	, m_mesh{ nullptr }
	, m_halfSpace{}
	, m_openLoops{ nullptr }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("PlaneNormal", m_halfSpace.GetPlaneNormalParam());
	AddParamBinding<ParamMode::In, ParamType::Float>("PlaneDist", m_halfSpace.GetPlaneDistParam());
	AddParamBinding<ParamMode::Out, ParamType::MultiIndices>("OpenLoops", m_openLoops);
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
			data::Triangle nt{ triVerts[0], triVerts[1], triVerts[2] };

			const glm::vec3 nn = nt.CalcNormal(m_mesh->vertices);
			const glm::vec3 on = t.CalcNormal(m_mesh->vertices);
			const float proj = glm::dot(nn, on);
			if (proj < 0.0f)
			{
				nt.Flip();
			}

			m_mesh->triangles.push_back(nt);
		}
		else if (triVerts.size() == 4)
		{
			m_mesh->AddQuad(triVerts[0], triVerts[2], triVerts[1], triVerts[3]);
			auto back = --(m_mesh->triangles.end());
			data::Triangle& nt2 = *back;
			data::Triangle& nt = *(--back);

			const glm::vec3 nn = nt.CalcNormal(m_mesh->vertices);
			const glm::vec3 on = t.CalcNormal(m_mesh->vertices);
			const float proj = glm::dot(nn, on);
			if (proj < 0.0f)
			{
				nt.Flip();
				nt2.Flip();
			}

		}
		else
		{
			Log().Warning("Triangle unexpectedly cut into %d pieces", static_cast<int>(triVerts.size()));
		}
	}

	// remove all unused vertices
	std::vector<uint32_t> newIdx(m_mesh->vertices.size(), 0xffffffff);
	for (auto const& t : m_mesh->triangles)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			newIdx.at(t[i]) = 0;
		}
	}
	uint32_t idx = 0;
	for (uint32_t& i : newIdx)
	{
		if (i == 0)
		{
			i = idx++;
		}
	}
	std::vector<glm::vec3> nv(idx);
	for (size_t i = 0; i < m_mesh->vertices.size(); ++i)
	{
		if (newIdx.at(i) == 0xffffffff) continue;
		nv.at(newIdx[i]) = m_mesh->vertices.at(i);
	}
	std::swap(m_mesh->vertices, nv);
	for (auto& t : m_mesh->triangles)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			t[i] = newIdx.at(t[i]);
			assert(t[i] < 0xffffffff);
		}
	}

	// finally collect open edges and build closed plane surface
	const std::unordered_set<data::HashableEdge> openEdges = m_mesh->CollectOpenEdges();
	algo::LoopsFromEdges(openEdges, m_openLoops, Log());

	float minX = 0.0f;
	auto [projX, projY] = m_halfSpace.Make2DCoordSys();
	std::unordered_map<uint32_t, glm::vec2> pt2d;
	for (auto loop : *m_openLoops)
	{
		for (uint32_t vi : *loop)
		{
			if (pt2d.contains(vi)) continue;
			const glm::vec3 v = m_mesh->vertices.at(vi) - m_halfSpace.Plane();
			const float x = glm::dot(v, projX);
			pt2d.insert(std::make_pair(vi, glm::vec2(x, glm::dot(v, projY))));
			if (x < minX)
			{
				minX = x;
			}
		}
	}

	{
		// do a constraint delauney triangulation, enforcing all edges
		// then only keep triangles by even-odd rule

		// use CGAL implementation:
		typedef CGAL::Single_precision_epick K;
		typedef K::Point_2 Point;
		typedef uint32_t VertexInfo;
		typedef CGAL::Triangulation_vertex_base_with_info_2<VertexInfo, K> Vb;
		typedef CGAL::Constrained_triangulation_face_base_2<K> Fb;
		typedef CGAL::Triangulation_data_structure_2<Vb, Fb> TDS;
		typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS> CDT;

		CDT cdt;

		for (auto const& edge : openEdges)
		{
			auto const& v0 = pt2d.at(edge.i0);
			auto const& v1 = pt2d.at(edge.i1);

			auto vh0 = cdt.insert(Point(v0.x, v0.y)); vh0->info() = edge.i0;
			auto vh1 = cdt.insert(Point(v1.x, v1.y)); vh1->info() = edge.i1;

			cdt.insert_constraint(vh0, vh1);
		}
		if (!cdt.is_valid())
		{
			Log().Error("Constrained Delaunay triangulation of open edge loops failed");
			return false;
		}

		for (auto fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit) {
			const uint32_t i0 = fit->vertex(0)->info();
			const uint32_t i1 = fit->vertex(1)->info();
			const uint32_t i2 = fit->vertex(2)->info();

			glm::vec2 c = (pt2d.at(i0) + pt2d.at(i1) + pt2d.at(i2));
			c /= glm::vec2(3.0f);
			glm::vec2 c2{ minX - 1.0f, c.y };
			bool inside = false;
			for (auto const& edge : openEdges)
			{
				if (segments_intersect(c, c2, pt2d.at(edge.i0), pt2d.at(edge.i1)))
				{
					inside = !inside;
				}
			}
			if (!inside) continue;

			data::Triangle t{ i0, i1, i2 };
			const glm::vec3 n = t.CalcNormal(m_mesh->vertices);
			const float p = glm::dot(n, m_halfSpace.Normal());
			if (p > 0.0f) t.Flip();
			m_mesh->triangles.push_back(t);
		}
	}

	return true;
}
