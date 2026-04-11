#include "SplitByEdges.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::compute;

SplitByEdges::SplitByEdges(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::Out, ParamType::MeshList>("Segments", m_segments);
	AddParamBinding<ParamMode::In, ParamType::Float>("Angle", m_angleDeg);
}

bool SplitByEdges::Invoke()
{
	if (!m_mesh)
	{
		Log().Error(L"'Mesh' not set");
		return false;
	}
	m_segments = std::make_shared<std::vector<std::shared_ptr<data::Mesh>>>();
	const float angleRad = glm::radians(std::max<float>(1.0f, m_angleDeg));

	std::vector<glm::vec3> fn;
	fn.resize(m_mesh->triangles.size());
	std::transform(m_mesh->triangles.begin(), m_mesh->triangles.end(), fn.begin(), [&v = m_mesh->vertices](data::Triangle& t) { return t.CalcNormal(v); });

	std::unordered_set<data::HashableEdge> edges;
	std::unordered_map<data::HashableEdge, std::array<uint32_t, 2>> ett;
	{
		constexpr std::array<glm::vec3, 2> emptyNormals{ glm::vec3{ 0.0f }, glm::vec3{ 0.0f } };
		constexpr std::array<uint32_t, 2> emptyIndices{ -1, -1 };
		std::unordered_map<data::HashableEdge, std::array<glm::vec3, 2>> en;
		for (size_t ti = 0; ti < m_mesh->triangles.size(); ++ti)
		{
			glm::vec3 tn = fn.at(ti);
			const data::Triangle& t = m_mesh->triangles.at(ti);
			for (uint32_t ei = 0; ei < 3; ++ei)
			{
				const data::HashableEdge he = t.HashableEdge(ei);
				auto enip = en.try_emplace(he, emptyNormals);
				enip.first->second.at(enip.second ? 0 : 1) = tn;
				auto ettip = ett.try_emplace(he, emptyIndices);
				ettip.first->second.at(ettip.second ? 0 : 1) = static_cast<uint32_t>(ti);
			}
		}
		Log().Detail("Total of %d edges", en.size());

		for (const auto& ep : en)
		{
			bool select = true;
			const auto& n2 = ep.second.at(1);
			if ((std::max)(std::abs(n2.x), (std::max)(std::abs(n2.y), std::abs(n2.z))) > 0.0001f)
			{
				float angle = glm::angle(ep.second.at(0), n2);
				select = (angle >= angleRad);
			}
			if (!select) continue;
			edges.insert(ep.first);
		}
	}
	Log().Detail("Selected %d edges above angle threshold", edges.size());

	std::unordered_set<size_t> freeTI;
	std::unordered_set<size_t> segmentTI;
	std::vector<size_t> addTI;
	freeTI.reserve(m_mesh->triangles.size());
	segmentTI.reserve(m_mesh->triangles.size() / 10);
	for (size_t ti = 0; ti < m_mesh->triangles.size(); ++ti)
	{
		freeTI.insert(ti);
	}

	auto const finalizeVertices = [](data::Mesh& tar, const data::Mesh& src)
	{
		std::unordered_map<uint32_t, uint32_t> vmap;
		vmap.reserve(tar.triangles.size());
		for (const auto& t : tar.triangles)
		{
			for (size_t i = 0; i < 3; ++i)
			{
				vmap.try_emplace(t[i], static_cast<uint32_t>(vmap.size()));
			}
		}
		tar.vertices.resize(vmap.size());
		for (auto& t : tar.triangles)
		{
			for (size_t i = 0; i < 3; ++i)
			{
				t[i] = vmap.at(t[i]);
			}
		}
		for (auto& vi : vmap)
		{
			tar.vertices.at(vi.second) = src.vertices.at(vi.first);
		}
	};

	size_t segCnt = 0;

	while (!freeTI.empty())
	{
		segmentTI.clear();
		size_t ti = *freeTI.begin();
		freeTI.erase(ti);
		addTI.push_back(ti);

		while (!addTI.empty())
		{
			ti = addTI.back();
			addTI.pop_back();
			segmentTI.insert(ti);

			for (uint32_t ei = 0; ei < 3; ++ei)
			{
				const auto& he = m_mesh->triangles.at(ti).HashableEdge(ei);
				if (edges.contains(he)) continue;

				const auto& tidxx = ett.at(he);
				uint32_t tj = tidxx.at((tidxx.at(0) == ti) ? 1 : 0);
				if (freeTI.contains(tj))
				{
					freeTI.erase(tj);
					addTI.push_back(tj);
				}
			}
		}

		std::shared_ptr<data::Mesh> m = std::make_shared<data::Mesh>();
		m->triangles.reserve(segmentTI.size());
		for (auto ti : segmentTI)
		{
			m->triangles.push_back(m_mesh->triangles.at(ti));
		}

		finalizeVertices(*m, *m_mesh);

		m_segments->push_back(m);
		segCnt++;
	}
	Log().Detail("Mesh split into %d segments", segCnt);

	return true;
}
