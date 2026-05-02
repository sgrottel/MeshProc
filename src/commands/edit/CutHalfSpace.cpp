#include "CutHalfSpace.h"

#include "utilities/Constrained2DTriangulation.h"
#include "utilities/LoopsFromEdges.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::edit;

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

	static float cross(const glm::vec2& u, const glm::vec2& v) {
		return u.x * v.y - u.y * v.x;
	}

	glm::vec2 intersect(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& d)
	{
		glm::vec2 r = b - a;
		glm::vec2 s = d - c;
		float denom = cross(r, s);
		assert(denom != 0.0f); // nonzero because they intersect
		float t = cross(c - a, s) / denom;
		return a + t * r;
	}

	glm::vec2 prec(const glm::vec2& value, float scale = 0.0001f)
	{
		return {
			std::round(value.x / scale) * scale,
			std::round(value.y / scale) * scale
		};
	}

}

CutHalfSpace::CutHalfSpace(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
	, m_mesh{ nullptr }
	, m_halfSpace{ std::make_shared<data::HalfSpace>() }
	, m_openLoops{ nullptr }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::HalfSpace>("HalfSpace", m_halfSpace);
	AddParamBinding<ParamMode::Out, ParamType::IndexListList>("OpenLoops", m_openLoops);
}

bool CutHalfSpace::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh not set");
		return false;
	}
	if (!m_halfSpace)
	{
		Log().Error("HalfSpace not set");
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
			const size_t j = (i + 1) % 3;
			const float distI = dist[t[i]];
			const float distJ = dist[t[j]];
			if (distI >= 0.0f)
			{
				if (std::find(triVerts.begin(), triVerts.end(), t[i]) == triVerts.end())
				{
					triVerts.push_back(t[i]);
				}
			}
			if ((distI >= 0.0f && distJ < 0.0f) || (distI < 0.0f && distJ >= 0.0f))
			{
				const data::HashableEdge he{ t[i], t[j] };
				uint32_t nvIdx;
				if (newVert.contains(he))
				{
					nvIdx = newVert.at(he);
				}
				else
				{
					glm::vec3 nv = m_halfSpace->CutInterpolate(he, dist, m_mesh->vertices);
					nvIdx = static_cast<uint32_t>(m_mesh->vertices.size());
					for (size_t nvI = 0; nvI < m_mesh->vertices.size(); ++nvI)
					{
						if (m_mesh->vertices.at(nvI) == nv)
						{
							nvIdx = static_cast<uint32_t>(nvI);
							break;
						}
					}
					if (nvIdx == static_cast<uint32_t>(m_mesh->vertices.size()))
					{
						m_mesh->vertices.push_back(nv);
					}
					newVert.insert(std::make_pair(he, nvIdx));
				}
				if (std::find(triVerts.begin(), triVerts.end(), nvIdx) == triVerts.end())
				{
					triVerts.push_back(nvIdx);
				}
			}
		}

		if (triVerts.size() == 3)
		{
			m_mesh->triangles.push_back(data::Triangle{ triVerts[0], triVerts[1], triVerts[2] });
		}
		else if (triVerts.size() == 4)
		{
			m_mesh->triangles.push_back(data::Triangle{ triVerts[0], triVerts[1], triVerts[2] });
			m_mesh->triangles.push_back(data::Triangle{ triVerts[2], triVerts[3], triVerts[0] });
		}
		else
		{
			Log().Warning("Triangle unexpectedly cut into %d pieces", static_cast<int>(triVerts.size()));
		}
	}

	m_mesh->RemoveIsolatedVertices();

	// finally collect open edges and build closed plane surface
	std::unordered_set<data::HashableEdge> openEdges = m_mesh->CollectOpenEdges();
	utilities::LoopsFromEdges(openEdges, m_openLoops, Log());

	{ // only keep loops that are entirely within the cutting plane
		size_t sizeBefore = m_openLoops->size();
		for (int32_t li = static_cast<int32_t>(m_openLoops->size()) - 1; li >= 0; --li)
		{
			bool inPlane = true;
			for (uint32_t vi : *m_openLoops->at(li))
			{
				float d = std::abs(m_halfSpace->Dist(m_mesh->vertices.at(vi)));
				if (d > 0.001f)
				{
					inPlane = false;
					break;
				}
			}

			if (!inPlane)
			{
				m_openLoops->erase(m_openLoops->begin() + li);
			}
		}
		if (sizeBefore != m_openLoops->size())
		{
			openEdges.clear();
			for (auto loop : *m_openLoops)
			{
				uint32_t prev = loop->back();
				for (uint32_t vi : *loop)
				{
					openEdges.insert(data::HashableEdge{ prev, vi });
					prev = vi;
				}
			}
		}
	}

	float minX = 0.0f;
	auto [projX, projY] = m_halfSpace->Make2DCoordSys();
	std::unordered_map<uint32_t, glm::vec2> pt2d;
	for (auto loop : *m_openLoops)
	{
		for (uint32_t vi : *loop)
		{
			if (pt2d.contains(vi)) continue;
			const glm::vec3 v = m_mesh->vertices.at(vi) - m_halfSpace->Plane();
			const float x = glm::dot(v, projX);
			pt2d.insert(std::make_pair(vi, glm::vec2(x, glm::dot(v, projY))));
			if (x < minX)
			{
				minX = x;
			}
		}
	}

	{
		utilities::Constrained2DTriangulation cvt(pt2d, openEdges, Log());
		auto allTries = cvt.Compute();
		if (cvt.HasError())
		{
			return false;
		}

		std::unordered_set<glm::vec2> intersections;

		for (auto cvtFace : allTries) {
			const uint32_t i0 = cvtFace.x;
			const uint32_t i1 = cvtFace.y;
			const uint32_t i2 = cvtFace.z;

			glm::vec2 c = (pt2d.at(i0) + pt2d.at(i1) + pt2d.at(i2));
			c /= glm::vec2(3.0f);

			glm::vec2 c2{ minX - 1.0f, c.y };

			intersections.clear();
			for (auto const& edge : openEdges)
			{
				const auto& p1 = pt2d.at(edge.i0);
				const auto& p2 = pt2d.at(edge.i1);
				if (segments_intersect(c, c2, p1, p2))
				{
					intersections.insert(prec(intersect(c, c2, p1, p2)));
				}
			}
			if (intersections.size() % 2 == 0) {
				continue;
			}

			data::Triangle t{ i0, i1, i2 };
			const glm::vec3 n = t.CalcNormal(m_mesh->vertices);
			const float p = glm::dot(n, m_halfSpace->Normal());
			if (p > 0.0f) t.Flip();
			m_mesh->triangles.push_back(t);
		}
	}

	return true;
}
