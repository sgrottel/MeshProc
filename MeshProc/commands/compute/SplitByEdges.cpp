#include "SplitByEdges.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

#include <array>
#include <unordered_map>
#include <unordered_set>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::compute;

SplitByEdges::SplitByEdges(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::InOut, ParamType::Scene>("Scene", m_scene);
	AddParamBinding<ParamMode::In, ParamType::Float>("Angle", m_angleDeg);
}

bool SplitByEdges::Invoke()
{
	if (!m_mesh)
	{
		Log().Error(L"'Mesh' not set");
		return false;
	}
	if (!m_scene)
	{
		m_scene = std::make_shared<data::Scene>();
	}
	const float angleRad = glm::radians(std::max<float>(1.0f, m_angleDeg));

	std::vector<glm::vec3> fn;
	fn.resize(m_mesh->triangles.size());
	std::transform(m_mesh->triangles.begin(), m_mesh->triangles.end(), fn.begin(), [&v = m_mesh->vertices](data::Triangle& t) { return t.CalcNormal(v); });

	std::unordered_set<data::HashableEdge> edges;
	{
		constexpr std::array<glm::vec3, 2> emptyNormals{ glm::vec3{ 0.0f }, glm::vec3{ 0.0f } };
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

	std::shared_ptr<data::Mesh> m = std::make_shared<data::Mesh>();
	m->triangles.reserve(edges.size() * 2);
	for (const auto& t : m_mesh->triangles)
	{
		const bool sel = edges.contains(t.HashableEdge(0))
			|| edges.contains(t.HashableEdge(1))
			|| edges.contains(t.HashableEdge(2));
		if (sel)
		{
			m->triangles.push_back(t);
		}
	}

	std::unordered_map<uint32_t, uint32_t> vmap;
	vmap.reserve(m->triangles.size());
	for (const auto& t : m->triangles)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			vmap.try_emplace(t[i], vmap.size());
		}
	}
	m->vertices.resize(vmap.size());
	for (auto& t : m->triangles)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			t[i] = vmap.at(t[i]);
		}
	}
	for (auto& vi : vmap)
	{
		m->vertices.at(vi.second) = m_mesh->vertices.at(vi.first);
	}

	m_scene->m_meshes.push_back(std::make_pair(m, glm::mat4{1.0f}));

	return true;
}
