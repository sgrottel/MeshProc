#include "LinearExtrude2DMesh.h"

#include "data/Mesh.h"
#include "data/Shape2D.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Constrained_triangulation_face_base_2.h>
#include <CGAL/Triangulation_data_structure_2.h>

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_map>
#include <vector>

using namespace meshproc;
using namespace meshproc::generator;

namespace
{
	struct Edge {
		size_t a, b;
		float minX, maxX, minY, maxY;
	};

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

LinearExtrude2DMesh::LinearExtrude2DMesh(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Float>("MinZ", m_minZ);
	AddParamBinding<ParamMode::In, ParamType::Float>("MaxZ", m_maxZ);
	AddParamBinding<ParamMode::In, ParamType::Shape2D>("Shape2D", m_shape2D);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("Mesh", m_mesh);
}

bool LinearExtrude2DMesh::Invoke()
{
	if (!m_shape2D)
	{
		Log().Error("Shape2D not set");
		return false;
	}

	return SimpleImpl();
/*
	m_mesh = std::make_shared<data::Mesh>();

	const float minZ = (std::min)(m_minZ, m_maxZ);
	const float maxZ = (std::max)(m_minZ, m_maxZ);

	std::unordered_map<glm::vec2, size_t> verts2dRev;
	std::vector<Edge> edges;

	const auto Vert2dIndex = [&](const glm::vec2& v)
		{
			auto it = verts2dRev.find(v);
			if (it == verts2dRev.end())
			{
				auto a = verts2dRev.insert(std::make_pair(v, verts2dRev.size()));
				it = a.first;
			}
			return it->second;
		};

	// flatten list of all edges and prepare clean indices to de-duplicated vertices
	for (const auto& loop : m_shape2D->loops)
	{
		const size_t loopSize = loop.second.size();
		if (loopSize < 3) continue;

		Vert2dIndex(loop.second[1]);
		for (size_t i = 0; i < loopSize; ++i)
		{
			edges.push_back(Edge{
				.a = Vert2dIndex(loop.second[i]),
				.b = Vert2dIndex(loop.second[(i + 1) % loopSize])
				});
		}
	}

	// resolve intersection of edges
	std::vector<glm::vec2> vs2;
	vs2.resize(verts2dRev.size());
	for (const auto& vr : verts2dRev)
	{
		vs2[vr.second] = vr.first;
	}
	for (auto& edge : edges)
	{
		auto const& p1 = vs2[edge.a];
		auto const& p2 = vs2[edge.b];
		edge.minX = (std::min)(p1.x, p2.x);
		edge.maxX = (std::max)(p1.x, p2.x);
		edge.minY = (std::min)(p1.y, p2.y);
		edge.maxY = (std::max)(p1.y, p2.y);
	}
	std::sort(edges.begin(), edges.end(), [](const auto& a, const auto& b) { return a.minX < b.minX; });
	// TODO: Implement

	// 2d face triangulation
	// TODO: Implement

	// genrate 3d mesh
	// TODO: Implement

	return false;
*/
}

bool LinearExtrude2DMesh::SimpleImpl()
{
	m_mesh = std::make_shared<data::Mesh>();

	const float minZ = (std::min)(m_minZ, m_maxZ);
	const float maxZ = (std::max)(m_minZ, m_maxZ);

	auto add = [&](glm::vec2 const& v, float z)
		{
			const size_t i = m_mesh->vertices.size();
			m_mesh->vertices.push_back(glm::vec3{ v, z });
			return i;
		};

	size_t vertexOffset = 0;
	for (auto const& loop : m_shape2D->loops)
	{
		if (loop.second.size() < 2) continue;

		if (loop.second.size() == 2)
		{
			m_mesh->AddQuad(
				add(loop.second.at(0), minZ),
				add(loop.second.at(1), minZ),
				add(loop.second.at(0), maxZ),
				add(loop.second.at(1), maxZ));
			vertexOffset += 4;
			continue;
		}

		const size_t ls = loop.second.size();

		std::vector<data::Triangle> capMesh;
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

			float minX = loop.second.front().x;

			for (size_t i = 0; i < ls; ++i)
			{
				auto const& v0 = loop.second.at(i);
				auto const& v1 = loop.second.at((i + 1) % ls);

				if (minX > v0.x) minX = v0.x;
				// v1 will be checked in next iteration anyway

				auto vh0 = cdt.insert(Point(v0.x, v0.y));
				vh0->info() = i;
				auto vh1 = cdt.insert(Point(v1.x, v1.y));
				vh1->info() = (i + 1) % ls;

				cdt.insert_constraint(vh0, vh1);
			}

			minX -= 1.0f;

			if (!cdt.is_valid())
			{
				Log().Error("Constrained Delaunay triangulation of open edge loops failed");
				return false;
			}

			for (auto fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit) {
				const uint32_t i0 = fit->vertex(0)->info();
				const uint32_t i1 = fit->vertex(1)->info();
				const uint32_t i2 = fit->vertex(2)->info();
				if (i0 == i1 || i0 == i2 || i1 == i2) continue;

				glm::vec2 c = (loop.second.at(i0) + loop.second.at(i1) + loop.second.at(i2));
				c /= glm::vec2(3.0f);
				glm::vec2 c2{ minX - 1.0f, c.y };
				bool inside = false;
				for (size_t i = 0; i < ls; ++i)
				{
					auto const& v0 = loop.second.at(i);
					auto const& v1 = loop.second.at((i + 1) % ls);

					if (segments_intersect(c, c2, v0, v1))
					{
						inside = !inside;
					}
				}
				if (!inside) continue;

				capMesh.push_back(data::Triangle{ i0, i1, i2 });
			}
		}

		const size_t ti1 = m_mesh->triangles.size();
		for (size_t i = 0; i < ls; ++i)
		{
			glm::vec2 const& v2 = loop.second.at(i);
			add(v2, minZ);
			add(v2, maxZ);
			m_mesh->AddQuad(
				vertexOffset + i * 2,
				vertexOffset + ((i + 1) % ls) * 2,
				vertexOffset + i * 2 + 1,
				vertexOffset + ((i + 1) % ls) * 2 + 1);
		}

		// caps
		const size_t ti2 = m_mesh->triangles.size();
		for (data::Triangle t : capMesh)
		{
			for (size_t i = 0; i < 3; ++i)
			{
				t[i] = vertexOffset + t[i] * 2;
			}
			auto const n = t.CalcNormal(m_mesh->vertices);
			if (n.z > 0.0) t.Flip();
			m_mesh->triangles.push_back(t);

			for (size_t i = 0; i < 3; ++i)
			{
				t[i]++;
			}
			t.Flip();
			m_mesh->triangles.push_back(t);
		}
		const size_t ti3 = m_mesh->triangles.size();

		// side orientation
		if (ti1 < ti2 && ti2 < ti3)
		{
			data::Triangle& const sideProbe = m_mesh->triangles.at(ti1);
			bool flip = false;
			for (size_t ti = ti2; ti < ti3; ++ti)
			{
				data::Triangle& const capProbe = m_mesh->triangles.at(ti);
				auto const ce = capProbe.CommonEdge(sideProbe);
				if (ce.x == ce.y) continue;

				flip = !capProbe.OrientationMatches(m_mesh->vertices, sideProbe, ce);
				break;
			}

			if (flip)
			{
				for (size_t ti = ti1; ti < ti2; ++ti)
				{
					m_mesh->triangles.at(ti).Flip();
				}
			}
		}

		vertexOffset += ls * 2;

	}

	return true;
}
