#include "SphereIco.h"

#include <SimpleLog/SimpleLog.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_set>

using namespace meshproc;

generator::SphereIco::SphereIco(const sgrottel::ISimpleLog& log)
	: Icosahedron(log)
{
	Iterations.Put() = 1;
}

bool generator::SphereIco::Invoke()
{
	if (!Icosahedron::Invoke())
	{
		return false;
	}

	std::shared_ptr<::Mesh> mesh = Mesh.Get();
	if (!mesh)
	{
		Log().Error("Mesh from Icosahedron not set");
		return false;
	}

	std::vector<Triangle> newTris;

	for (uint32_t i = 0; i < Iterations.Get(); ++i)
	{
		std::unordered_map<glm::uvec2, uint32_t> edges;

		for (Triangle const& t : mesh->triangles)
		{
			for (int i = 0; i < 3; ++i)
			{
				glm::uvec2 e{ t.HashableEdge(i) };
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

		for (Triangle const& t : mesh->triangles)
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
