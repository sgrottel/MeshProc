#include "CutPlaneLoop.h"

#include "utilities/Constrained2DTriangulation.h"
#include "utilities/LoopsFromEdges.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>

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
	std::unordered_map<uint32_t, glm::vec2> pt2d;
	float minX = 0.0f;
	{
		// collect all loops on the plane
		std::shared_ptr<std::vector<std::shared_ptr<std::vector<uint32_t>>>> openLoops;
		utilities::LoopsFromEdges(edges, openLoops, Log());

		auto [projX, projY] = m_plane->Make2DCoordSys();
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
	std::unordered_map<uint32_t, uint32_t> toNegLoopVert;
	loopNeg.reserve(loop.size());
	toNegLoopVert.reserve(loop.size());
	m_mesh->vertices.reserve(m_mesh->vertices.size() + loop.size());
	for (uint32_t vi : loop)
	{
		const uint32_t nvi = static_cast<uint32_t>(m_mesh->vertices.size());
		loopNeg.push_back(nvi);
		toNegLoopVert.insert({ vi, nvi });
		m_mesh->vertices.push_back(m_mesh->vertices.at(vi));
	}

	{
		std::unordered_set<data::HashableEdge> loopEdges;
		uint32_t pvi = loop.back();
		for (uint32_t vi : loop)
		{
			loopEdges.insert({ pvi, vi });
			pvi = vi;
		}

		utilities::Constrained2DTriangulation cap{ pt2d, loopEdges, Log() };
		std::vector<glm::uvec3> capTris = cap.Compute();
		if (cap.HasError())
		{
			return false;
		}

		std::unordered_set<glm::vec2> intersections;
		m_mesh->triangles.reserve(m_mesh->triangles.size() + capTris.size() * 2);
		for (glm::uvec3 rt : capTris)
		{
			const uint32_t i0 = rt.x;
			const uint32_t i1 = rt.y;
			const uint32_t i2 = rt.z;

			glm::vec2 c = (pt2d.at(i0) + pt2d.at(i1) + pt2d.at(i2));
			c /= glm::vec2(3.0f);

			glm::vec2 c2{ minX - 1.0f, c.y };

			intersections.clear();
			uint32_t pvidx = loop.back();
			for (uint32_t vidx : loop)
			{
				const data::HashableEdge edge{ pvidx, vidx };
				pvidx = vidx;
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

			data::Triangle t{ rt.x, rt.y, rt.z };
			const glm::vec3 tn = t.CalcNormal(m_mesh->vertices);
			if (glm::dot(tn, m_plane->Normal()) > 0.0f)
			{
				t.Flip();
			}

			m_mesh->triangles.push_back(t); // tri of pos-loop cap

			t[0] = toNegLoopVert.at(t[0]);
			t[1] = toNegLoopVert.at(t[1]);
			t[2] = toNegLoopVert.at(t[2]);
			t.Flip();
			m_mesh->triangles.push_back(t); // tri of neg-loop cap
		}
	}

	// cut triangles
	for (uint32_t ti : tris)
	{
		data::Triangle t = m_mesh->triangles.at(ti);
		const float d[3] = { dist.at(t[0]), dist.at(t[1]), dist.at(t[2]) };
		const bool hasNeg = d[0] < 0.0f || d[1] < 0.0f || d[2] < 0.0f;
		const bool hasNul = d[0] == 0.0f || d[1] == 0.0f || d[2] == 0.0f;
		const bool hasPos = d[0] > 0.0f || d[1] > 0.0f || d[2] > 0.0f;

		if (hasPos && !hasNeg)
		{
			// touching, keep as is
			m_mesh->triangles.push_back(t);
			continue;
		}
		if (!hasPos && hasNeg)
		{
			// touching from neg, change touching vertices, then keep
			for (uint32_t i = 0; i < 3; ++i)
			{
				if (d[i] == 0.0f)
				{
					t[i] = toNegLoopVert.at(t[i]);
				}
			}
			m_mesh->triangles.push_back(t);
			continue;
		}

		// tri is really to be split
		std::vector<uint32_t> posLoop;
		posLoop.reserve(4);
		std::vector<uint32_t> negLoop;
		negLoop.reserve(4);
		for (uint32_t i = 0; i < 3; ++i)
		{
			if (d[i] == 0.0f)
			{
				posLoop.push_back(t[i]);
				negLoop.push_back(toNegLoopVert.at(t[i]));
				continue;
			}
			if (d[i] < 0.0f)
			{
				negLoop.push_back(t[i]);
			}
			if (d[i] > 0.0f)
			{
				posLoop.push_back(t[i]);
			}
			const auto cutVertIt = newVert.find(data::HashableEdge{ t[i], t[(i + 1) % 3] });
			if (cutVertIt == newVert.end())
			{
				continue;
			}
			posLoop.push_back(cutVertIt->second);
			negLoop.push_back(toNegLoopVert.at(cutVertIt->second));
		}

		if (posLoop.size() == 3)
		{
			m_mesh->triangles.push_back(data::Triangle{ posLoop[0], posLoop[1], posLoop[2] });
		}
		else if (posLoop.size() == 4)
		{
			m_mesh->triangles.push_back(data::Triangle{ posLoop[0], posLoop[1], posLoop[2] });
			m_mesh->triangles.push_back(data::Triangle{ posLoop[2], posLoop[3], posLoop[0] });
		}
		else
		{
			Log().Warning("Triangle unexpectedly cut into %d pieces in pos halfspace", static_cast<int>(posLoop.size()));
		}

		if (negLoop.size() == 3)
		{
			m_mesh->triangles.push_back(data::Triangle{ negLoop[0], negLoop[1], negLoop[2] });
		}
		else if (negLoop.size() == 4)
		{
			m_mesh->triangles.push_back(data::Triangle{ negLoop[0], negLoop[1], negLoop[2] });
			m_mesh->triangles.push_back(data::Triangle{ negLoop[2], negLoop[3], negLoop[0] });
		}
		else
		{
			Log().Warning("Triangle unexpectedly cut into %d pieces in neg halfspace", static_cast<int>(negLoop.size()));
		}
	}

	// erase old, cut triangles
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

	// cleanup
	m_mesh->RemoveIsolatedVertices();

	// done
	return true;
}
