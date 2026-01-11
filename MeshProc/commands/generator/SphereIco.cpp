#include "SphereIco.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_map>

using namespace meshproc;
using namespace meshproc::commands;

generator::SphereIco::SphereIco(const sgrottel::ISimpleLog& log)
	: Icosahedron(log)
{
	AddParamBinding<ParamMode::In, ParamType::UInt32>("Iterations", m_iterations);
}

bool generator::SphereIco::Invoke()
{
	if (!Icosahedron::Invoke())
	{
		return false;
	}

	std::shared_ptr<data::Mesh> mesh = m_mesh;
	if (!mesh)
	{
		Log().Error("Mesh from Icosahedron not set");
		return false;
	}

	std::vector<data::Triangle> newTris;

	for (uint32_t i = 0; i < m_iterations; ++i)
	{
		std::unordered_map<data::HashableEdge, uint32_t> edges;

		for (data::Triangle const& t : mesh->triangles)
		{
			for (int i = 0; i < 3; ++i)
			{
				auto e{ t.HashableEdge(i) };
				if (edges.find(e) == edges.end())
				{
					edges.insert({ e, 0 });
				}
			}
		}
		mesh->vertices.reserve(mesh->vertices.size() + edges.size());

		for (auto& e : edges)
		{
			e.second = static_cast<uint32_t>(mesh->vertices.size());
			mesh->vertices.push_back(
				glm::normalize(
					(mesh->vertices[e.first.x] + mesh->vertices[e.first.y]) * 0.5f));
		}

		newTris.reserve(mesh->triangles.size() * 4);

		for (data::Triangle const& t : mesh->triangles)
		{
			uint32_t n[3]{
				edges[t.HashableEdge(0)],
				edges[t.HashableEdge(1)],
				edges[t.HashableEdge(2)]
			};

			newTris.push_back({ t[0], n[0], n[2] });
			newTris.push_back({ t[1], n[1], n[0] });
			newTris.push_back({ t[2], n[2], n[1] });
			newTris.push_back({ n[0], n[1], n[2] });
		}

		std::swap(mesh->triangles, newTris);
	}

	return true;
}
