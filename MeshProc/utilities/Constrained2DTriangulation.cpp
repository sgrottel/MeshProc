#include "Constrained2DTriangulation.h"

#include <SimpleLog/SimpleLog.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Constrained_triangulation_face_base_2.h>
#include <CGAL/Triangulation_data_structure_2.h>

using namespace meshproc;

utilities::Constrained2DTriangulation::Constrained2DTriangulation(
	const std::unordered_map<uint32_t, glm::vec2>& points,
	const std::unordered_set<data::HashableEdge>& edges,
	const sgrottel::ISimpleLog& log)
	: m_points{ points }
	, m_edges{ edges }
	, m_log{ log }
	, m_hasError{ false }
{
}

std::vector<glm::uvec3> utilities::Constrained2DTriangulation::Compute() const
{
	std::vector<glm::uvec3> result;

	m_hasError = true; // assume early exit

	// use CGAL implementation:
	typedef CGAL::Single_precision_epick K;
	typedef K::Point_2 Point;
	typedef uint32_t VertexInfo;
	typedef CGAL::Triangulation_vertex_base_with_info_2<VertexInfo, K> Vb;
	typedef CGAL::Constrained_triangulation_face_base_2<K> Fb;
	typedef CGAL::Triangulation_data_structure_2<Vb, Fb> TDS;
	typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS> CDT;

	CDT cdt;

	std::unordered_map<uint32_t, CDT::Vertex_handle> cdtVHs;
	for (auto const& p : m_points)
	{
		auto const& v = p.second;
		auto vh = cdt.insert(Point(v.x, v.y));
		vh->info() = p.first;
		cdtVHs.insert(std::make_pair(p.first, vh));
	}

	for (auto const& edge : m_edges)
	{
		cdt.insert_constraint(cdtVHs.at(edge.i0), cdtVHs.at(edge.i1));
	}
	if (!cdt.is_valid())
	{
		m_log.Error("Constrained Delaunay triangulation of open edge loops failed");
		result.clear();
		return result;
	}

	for (auto fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); ++fit) {
		const uint32_t i0 = fit->vertex(0)->info();
		const uint32_t i1 = fit->vertex(1)->info();
		const uint32_t i2 = fit->vertex(2)->info();

		result.push_back(glm::uvec3{ i0, i1, i2 });
	}

	m_hasError = false;
	return result;
}
