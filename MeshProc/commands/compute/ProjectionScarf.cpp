#include "ProjectionScarf.h"

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
						vm.insert(std::make_pair(tvi[j], static_cast<uint32_t>(flatVert.size())));
						flatVert.push_back(p);
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
							cem.insert(std::make_pair(he, static_cast<uint32_t>(flatVert.size())));
							flatVert.push_back(p);
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

	// TODO: Implement


	// 2. created projected edges

	// 3. further cut triangles by projected edges -> be aware of t-vertices in the projected edge

	// 4. created projected vertices

	// 5. scarf geometry

	// 6. scarf base hole close polygon

	m_outmesh = std::make_shared<data::Mesh>();

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
