#include "ProjectionScarf.h"

#include "utilities/Constrained2DTriangulation.h"
#include "utilities/Vec2Intersections.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <SimpleLog/SimpleLog.hpp>

#include <array>
#include <cassert>
#include <unordered_map>
#include <unordered_set>

using namespace meshproc;
using namespace meshproc::commands;

compute::ProjectionScarf::ProjectionScarf(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::HalfSpace>("Projection", m_projection);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("OutMesh", m_outmesh);
}

namespace
{

	glm::vec3 prec(const glm::vec3& value, float scale = 0.0001f)
	{
		return {
			std::round(value.x / scale) * scale,
			std::round(value.y / scale) * scale,
			std::round(value.z / scale) * scale
		};
	}

}

bool compute::ProjectionScarf::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_projection)
	{
		Log().Error("Mesh is empty");
		return false;
	}

	// select triangles (dot(tri.normal, proj.normal) > 0) & at least one vertex in positive half space
	std::vector<uint32_t> selTri;
	selTri.reserve(m_mesh->triangles.size() / 4);
	for (size_t ti = 0; ti < m_mesh->triangles.size(); ++ti)
	{
		const auto& t = m_mesh->triangles.at(ti);

		const glm::vec3 tn = t.CalcNormal(m_mesh->vertices);
		const float o = glm::dot(tn, m_projection->Normal());
		if (o < 0.0f) continue; // triangle is oriented against projection -> not relevant (asumption: watertight closed mesh)

		bool phs = false;
		for (size_t tvi = 0; tvi < 3; ++tvi)
		{
			const float d = m_projection->Dist(m_mesh->vertices.at(t[tvi]));
			if (d >= 0.0f)
			{
				phs = true;
				break;
			}
		}
		if (!phs) continue; // triangle is entirely in negative half space -> not relevant

		selTri.push_back(static_cast<uint32_t>(ti));
	}

	// project vertices (cut triangles where needed)
	std::vector<glm::vec3> flatVert;
	std::unordered_set<data::HashableEdge> flatEdges;
	std::unordered_map<glm::vec3, uint32_t> flatVertRevIdx;
	auto addFlatVert = [&](glm::vec3 p)
		{
			glm::vec3 probe = prec(p);
			if (!flatVertRevIdx.contains(probe))
			{
				flatVertRevIdx.insert(std::make_pair(probe, static_cast<uint32_t>(flatVert.size())));
				flatVert.push_back(p);
			}
			return flatVertRevIdx.at(probe);
		};
	{
		flatVert.reserve(selTri.size());
		std::unordered_map<uint32_t, uint32_t> vm;
		std::unordered_map<data::HashableEdge, uint32_t> cem;
		std::vector<uint32_t> triLoop;
		triLoop.reserve(4);
		for (uint32_t ti : selTri)
		{
			const std::array<uint32_t, 4> tvi{
				m_mesh->triangles.at(ti)[0],
				m_mesh->triangles.at(ti)[1],
				m_mesh->triangles.at(ti)[2]
			};
			const std::array<glm::vec3, 3> vec{
				m_mesh->vertices.at(tvi[0]),
				m_mesh->vertices.at(tvi[1]),
				m_mesh->vertices.at(tvi[2])
			};
			const std::array<float, 3> dist{
				m_projection->Dist(vec[0]),
				m_projection->Dist(vec[1]),
				m_projection->Dist(vec[2]),
			};

			triLoop.clear();
			for (size_t j = 0; j < 3; j++)
			{
				if (dist[j] >= 0.0)
				{
					if (!vm.contains(tvi[j]))
					{
						const glm::vec3 p = vec[j] - m_projection->Normal() * dist[j];
						vm.insert(std::make_pair(tvi[j], addFlatVert(p)));
					}
					triLoop.push_back(vm.at(tvi[j]));
				}
				else
				{
					for (size_t je = 2; je > 0; je--)
					{
						size_t j2 = (j + je) % 3;
						if (dist[j2] < 0.0) continue;

						data::HashableEdge he{ tvi[j], tvi[j2] };
						if (!cem.contains(he))
						{
							const float ad = std::abs(dist[j2]);
							const float bd = std::abs(dist[j]);
							const float a = ad / (ad + bd);
							const float b = bd / (ad + bd);
							const glm::vec3 p = vec[j] * a + vec[j2] * b;
							assert(m_projection->Dist(p) < 0.00001);
							cem.insert(std::make_pair(he, addFlatVert(p)));
						}
						triLoop.push_back(cem.at(he));
					}
				}
			}

			if (triLoop.size() > 1)
			{
				for (size_t j = 0; j < triLoop.size(); j++)
				{
					flatEdges.insert(data::HashableEdge{ triLoop.at(j), triLoop.at((j + 1) % triLoop.size()) });
				}
			}
		}
	}

	// prepare 2d coordinates
	std::unordered_map<uint32_t, glm::vec2> pt2d;
	auto [projX, projY] = m_projection->Make2DCoordSys();
	for (size_t vi = 0; vi < flatVert.size(); vi++)
	{
		const glm::vec3 v = flatVert.at(vi) - m_projection->Plane();
		const float x = glm::dot(v, projX);
		const float y = glm::dot(v, projY);
		pt2d.insert(std::make_pair(static_cast<uint32_t>(vi), glm::vec2(x, y)));
	}

	// resolve T-Vertices (will also resolve co-linear edges)
	{
		std::unordered_set<data::HashableEdge> toRm;
		std::unordered_set<data::HashableEdge> toAdd;
		do
		{
			toRm.clear();
			toAdd.clear();

			for (auto it1 = flatEdges.begin(); it1 != flatEdges.end(); ++it1)
			{
				const glm::vec2 a = pt2d.at(it1->i0);
				const glm::vec2 b = pt2d.at(it1->i1);

				const glm::vec2 d = glm::normalize(b - a);
				const float dd = glm::distance(a, b);
				const glm::vec2 c{ d.y, -d.x };

				for (auto pi = pt2d.begin(); pi != pt2d.end(); ++pi)
				{
					if (it1->Has(pi->first)) continue;
					const glm::vec2 p = pi->second - a;
					const float x = glm::dot(d, p);
					if (x < 0.0f || x >= dd) continue;
					const float y = std::abs(glm::dot(c, p));
					if (y > 0.0001f) continue;

					data::HashableEdge he = *it1;
					toRm.insert(he);
					toAdd.insert(data::HashableEdge{ he.i0, pi->first });
					toAdd.insert(data::HashableEdge{ he.i1, pi->first });
					break;
				}
			}

			if (!toRm.empty())
			{
				std::erase_if(flatEdges, [&toRm](data::HashableEdge const& he) { return toRm.contains(he); });
			}
			if (!toAdd.empty())
			{
				for (data::HashableEdge const& he : toAdd)
				{
					flatEdges.insert(he);
				}
			}
		} while (!toRm.empty() || !toAdd.empty());
	}

	// resolve flatEdge intersections in 2d
	{
		std::unordered_set<data::HashableEdge> of;
		for (auto it1 = flatEdges.begin(); it1 != flatEdges.end(); ++it1)
		{
			const auto& a = pt2d.at(it1->i0);
			const auto& b = pt2d.at(it1->i1);

			auto it2 = it1;
			it2++;
			for (; it2 != flatEdges.end(); ++it2)
			{
				if (it1->Touch(*it2)) continue;
				const auto& c = pt2d.at(it2->i0);
				const auto& d = pt2d.at(it2->i1);

				if (utilities::vec2segments::DoesIntersect(a, b, c, d))
				{
					of.insert(*it1);
					of.insert(*it2);
				}
			}
		}
		std::erase_if(flatEdges, [&of](const data::HashableEdge& e) { return of.contains(e); });

		std::unordered_set<data::HashableEdge> toRm;
		std::unordered_set<data::HashableEdge> toAdd;
		do
		{
			toRm.clear();
			toAdd.clear();

			for (auto it1 = of.begin(); it1 != of.end(); ++it1)
			{
				const auto& a = pt2d.at(it1->i0);
				const auto& b = pt2d.at(it1->i1);
				for (auto it2 = std::next(it1); it2 != of.end(); ++it2)
				{
					if (it1->Touch(*it2)) continue;
					const auto& c = pt2d.at(it2->i0);
					const auto& d = pt2d.at(it2->i1);
					if (utilities::vec2segments::DoesIntersect(a, b, c, d))
					{
						data::HashableEdge e1 = *it1;
						data::HashableEdge e2 = *it2;

						toRm.insert(e1);
						toRm.insert(e2);

						glm::vec2 d1 = glm::normalize(b - a);
						glm::vec2 d2 = glm::normalize(d - c);

						// TODO: still does not always work... still reaches co-linear pairs. Why?
						glm::vec2 e = utilities::vec2segments::CalcIntersection(a, b, c, d);

						uint32_t eIdx = addFlatVert(m_projection->Plane() + projX * e.x + projY * e.y);
						if (!pt2d.contains(eIdx))
						{
							pt2d.insert(std::make_pair(eIdx, e));
						}

						toAdd.insert(data::HashableEdge{ e1.i0, eIdx });
						toAdd.insert(data::HashableEdge{ e1.i1, eIdx });
						toAdd.insert(data::HashableEdge{ e2.i0, eIdx });
						toAdd.insert(data::HashableEdge{ e2.i1, eIdx });
					}
				}
			}

			if (!toRm.empty())
			{
				std::erase_if(of, [&toRm](data::HashableEdge const& he) { return toRm.contains(he); });
			}
			if (!toAdd.empty())
			{
				for (data::HashableEdge const& he : toAdd)
				{
					of.insert(he);
				}
			}
		} while (!toRm.empty() || !toAdd.empty());

		for (data::HashableEdge const& he : of)
		{
			flatEdges.insert(he);
		}
	}

	// compute plane triangulation base
	std::vector<data::Triangle> flatTri;
	{
		utilities::Constrained2DTriangulation cvt(pt2d, flatEdges, Log());
		auto allTries = cvt.Compute();
		if (cvt.HasError())
		{
			return false;
		}

		flatTri.resize(allTries.size());
		std::transform(
			allTries.begin(),
			allTries.end(),
			flatTri.begin(),
			[](const glm::uvec3& t)
			{
				return data::Triangle{ t.x, t.y, t.z };
			});
	}

	// TODO: Implement


	// 2. created projected edges

	// 3. further cut triangles by projected edges -> be aware of t-vertices in the projected edge

	// 4. created projected vertices

	// 5. scarf geometry

	// 6. scarf base hole close polygon

	m_outmesh = std::make_shared<data::Mesh>();
	m_outmesh->vertices = std::move(flatVert);
	m_outmesh->triangles = std::move(flatTri);

	//m_outmesh->vertices.resize(m_mesh->vertices.size());
	//std::copy(m_mesh->vertices.begin(), m_mesh->vertices.end(), m_outmesh->vertices.begin());
	////std::transform(
	////	m_mesh->vertices.begin(),
	////	m_mesh->vertices.end(),
	////	m_outmesh->vertices.begin(),
	////	[&](glm::vec3 const& v)
	////	{
	////		return v - m_projection->Normal() * m_projection->Dist(v);
	////	});

	//m_outmesh->triangles.resize(selTri.size());
	//std::transform(
	//	selTri.begin(),
	//	selTri.end(),
	//	m_outmesh->triangles.begin(),
	//	[&](uint32_t i)
	//	{
	//		return m_mesh->triangles.at(i);
	//	});

	//m_outmesh->RemoveIsolatedVertices();

	//for (glm::vec3 const& v : flatVert)
	//{
	//	m_outmesh->vertices.push_back(v);
	//}

	Log().Error("NOT IMPLEMENTED");
	return false;
}
