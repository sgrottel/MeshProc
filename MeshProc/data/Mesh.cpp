#include "Mesh.h"

#include <cmath>

using namespace meshproc;
using namespace meshproc::data;

bool Mesh::IsValid() const
{
	for (glm::vec3 const& v : vertices) {
		for (int i = 0; i < 3; ++i) {
			if (std::isnan(v[i])) return false;
		}
	}
	for (Triangle const& t : triangles) {
		for (int i = 0; i < 3; ++i) {
			if (t[i] >= vertices.size()) return false;
		}
		if (t[0] == t[1] || t[1] == t[2] || t[0] == t[2]) {
			return false;
		}
	}

	return true;
}

std::unordered_set<data::HashableEdge> Mesh::CollectOpenEdges() const
{
	std::unordered_set<data::HashableEdge> openEdges;
	for (data::Triangle const& t : triangles)
	{
		for (int i = 0; i < 3; ++i)
		{
			data::HashableEdge e{ t.HashableEdge(i) };
			if (openEdges.find(e) == openEdges.end())
			{
				openEdges.insert(e);
			}
			else
			{
				openEdges.erase(e);
			}
		}
	}
	return openEdges;
}

void Mesh::RemoveIsolatedVertices()
{
	std::unordered_set<uint32_t> uv;
	const uint32_t vertSize = static_cast<uint32_t>(vertices.size());

	uv.reserve(vertSize);
	for (uint32_t i = 0; i < vertSize; ++i)
	{
		uv.insert(i);
	}
	for (Triangle const& t : triangles)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			uv.erase(t[i]);
		}
	}

	if (uv.empty())
	{
		// no unused vertices
		return;
	}

	std::unordered_map<uint32_t, uint32_t> remap;
	remap.reserve(vertSize);
	uint32_t off = 0;
	for (uint32_t i = 0; i < vertSize; ++i)
	{
		if (uv.contains(i))
		{
			off++;
		}
		else
		{
			remap.insert(std::make_pair(i, i - off));
			if (off > 0)
			{
				vertices.at(i - off) = vertices.at(i);
			}
		}
	}
	vertices.resize(vertSize - uv.size());

	for (Triangle& t : triangles)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			t[i] = remap.at(t[i]);
		}
	}
}
