#include "Extract2DLoops.h"

#include <SimpleLog/SimpleLog.hpp>

#include "algo/LoopsFromEdges.h"
#include "data/HashableEdge.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <array>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace meshproc;

Extract2DLoops::Extract2DLoops(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
	, m_mesh{ nullptr }
	, m_halfSpace{}
	, m_loops{ nullptr }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("PlaneNormal", m_halfSpace.GetPlaneNormalParam());
	AddParamBinding<ParamMode::In, ParamType::Float>("PlaneDist", m_halfSpace.GetPlaneDistParam());
	AddParamBinding<ParamMode::Out, ParamType::Shape2D>("Loops", m_loops);
}

bool Extract2DLoops::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh not set");
		return false;
	}
	if (!m_halfSpace.ValidateParams(Log()))
	{
		return false;
	}

	auto [projX, projY] = m_halfSpace.Make2DCoordSys();

	std::unordered_map<glm::vec2, size_t> v2d;
	std::vector<data::HashableEdge> edges2d;

	for (auto const& t : m_mesh->triangles)
	{
		glm::vec3 v[3];
		float dist[3];
		bool pos[3]; // is v in pos halfspace
		for (int i = 0; i < 3; ++i)
		{
			pos[i] = (dist[i] = m_halfSpace.Dist(v[i] = m_mesh->vertices[t[i]])) >= 0.0f;
		}

		if (pos[0] == pos[1] && pos[0] == pos[2])
		{
			// triangle entirely in one halfspace
			continue;
		}
		// else triangle crosses halfspace plane

		std::unordered_set<glm::vec2> pts;

		// TODO: this can be improved by hashing not over the point's coordinates, but over the hashable-edges of the triangles instead.
		auto mkPt = [](float x, float y)
			{
				constexpr double prec = 256 * 1024.0;
				x = static_cast<float>(std::round(x * prec) / prec);
				y = static_cast<float>(std::round(y * prec) / prec);
				return glm::vec2{ x, y };
			};

		for (int i = 0; i < 3; ++i) {
			const int j = (i + 1) % 3;
			if (pos[i] == pos[j]) continue;
			const float adi = abs(dist[i]);
			const float adj = abs(dist[j]);
			const float b = adi / (adi + adj);
			const float a = adj / (adi + adj);
			const glm::vec3 nv = v[i] * a + v[j] * b;

			pts.insert(mkPt(glm::dot(nv, projX), glm::dot(nv, projY)));
		}

		if (pts.size() < 2)
		{
			// degenerated to single point
			continue;
		}
		if (pts.size() > 2)
		{
			Log().Warning("Strange triangle with !=2 projected points: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)",
				v[0].x, v[0].y, v[0].z, v[1].x, v[1].y, v[1].z, v[2].x, v[2].y, v[2].z);
			continue;
		}

		// I got edge!
		auto addV = [&](const glm::vec2& p)
			{
				if (auto c = v2d.find(p); c != v2d.end())
				{
					return static_cast<uint32_t>(c->second);
				}
				const auto n = v2d.insert(std::make_pair(p, v2d.size()));
				return static_cast<uint32_t>(n.first->second);
			};
		data::HashableEdge idx{ 0, 0 };
		std::transform(pts.begin(), pts.end(), idx.m_idx, addV);
		if (idx.i0 == idx.i1)
		{
			Log().Warning("Edge degenerated");
			continue;
		}
		edges2d.push_back(idx);
	}

	m_loops = std::make_shared<data::Shape2D>();

	std::vector<glm::vec2> vec2d;
	vec2d.resize(v2d.size());
	for (auto const& p : v2d) {
		vec2d[p.second] = p.first;
	}

	ParamTypeInfo_t<ParamType::MultiVertexSelection> loops;
	algo::LoopsFromEdges(edges2d, loops, Log());
	m_loops->loops.clear();
	for (auto const& l : *loops)
	{
		std::vector<glm::vec2> pl;
		pl.resize(l->size());
		std::transform(l->begin(), l->end(), pl.begin(), [&](size_t idx) { return vec2d.at(idx); });
		m_loops->loops.insert(std::make_pair(m_loops->loops.size(), std::move(pl)));
	}

	Log().Detail("Extracted %d loops", static_cast<int>(m_loops->loops.size()));

	return true;
}
