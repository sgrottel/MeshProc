#include "LinearExtrude2DMesh.h"

#include "data/Mesh.h"
#include "data/Shape2D.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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
}
