#include "CollapseTriangles.h"

#include <SimpleLog/SimpleLog.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>

using namespace meshproc;

CollapseTriangles::CollapseTriangles(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Indices>("Triangles", m_tris);
}

bool CollapseTriangles::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_tris)
	{
		Log().Error("Triangles is empty");
		return false;
	}

	std::vector<glm::vec3> verts;
	verts.reserve(m_mesh->vertices.size());

	constexpr const uint32_t invalid = 0xffffffff;
	std::vector<uint32_t> reVert(m_mesh->vertices.size(), invalid);

	for (auto const& triIdx : *m_tris)
	{
		auto& tri = m_mesh->triangles.at(triIdx);
		uint32_t setIdx = (std::min)((std::min)(reVert.at(tri[0]), reVert.at(tri[1])), reVert.at(tri[2]));
		if (setIdx == invalid)
		{
			setIdx = static_cast<uint32_t>(verts.size());
			verts.push_back(glm::vec3{ 0.0f });
		}
		for (int i = 0; i < 3; ++i)
		{
			reVert.at(tri[i]) = setIdx;
		}
	}

	std::vector<uint32_t> weight(verts.size(), 0);
	for (size_t i = 0; i < m_mesh->vertices.size(); ++i)
	{
		const uint32_t idx = reVert.at(i);
		if (idx == invalid)
		{
			reVert.at(i) = static_cast<uint32_t>(verts.size());
			verts.push_back(m_mesh->vertices.at(i));
		}
		else
		{
			verts.at(idx) += m_mesh->vertices.at(i);
			weight.at(idx)++;
		}
	}
	for (size_t i = 0; i < weight.size(); ++i)
	{
		verts.at(i) /= static_cast<float>(weight.at(i));
	}

	std::swap(m_mesh->vertices, verts);
	std::erase_if(
		m_mesh->triangles,
		[&](auto const& t)
		{
			const uint32_t ni0 = reVert.at(t[0]);
			const uint32_t ni1 = reVert.at(t[1]);
			const uint32_t ni2 = reVert.at(t[2]);
			return (ni0 == ni1) || (ni0 == ni2) || (ni1 == ni2);
		});

	for (auto& t : m_mesh->triangles)
	{
		for (int i = 0; i < 3; ++i)
		{
			t[i] = reVert.at(t[i]);
		}
	}

	// possible to have degenerate triangles
	bool hasIssue = false;
	std::unordered_map<glm::uvec3, unsigned int> triCnt;
	for (auto const& t : m_mesh->triangles)
	{
		glm::uvec3 rt{ t[0], t[1], t[2] };
		if (rt[0] > rt[1]) std::swap(rt[0], rt[1]);
		if (rt[0] > rt[2]) std::swap(rt[0], rt[2]);
		if (rt[1] > rt[2]) std::swap(rt[1], rt[2]);

		auto it = triCnt.find(rt);
		if (it == triCnt.end())
		{
			triCnt.insert(std::make_pair(rt, 1));
		}
		else
		{
			it->second++;
			hasIssue = true;
		}
	}
	if (hasIssue)
	{
		// remove all offending triangles
		std::erase_if(
			m_mesh->triangles,
			[&](auto const& t)
			{
				glm::uvec3 rt{ t[0], t[1], t[2] };
				if (rt[0] > rt[1]) std::swap(rt[0], rt[1]);
				if (rt[0] > rt[2]) std::swap(rt[0], rt[2]);
				if (rt[1] > rt[2]) std::swap(rt[1], rt[2]);
				return triCnt.at(rt) > 1;
			});

		// remove isolated vertices
		reVert.resize(m_mesh->vertices.size());
		std::fill(reVert.begin(), reVert.end(), invalid);
		uint32_t next = 0;
		for (auto& t : m_mesh->triangles)
		{
			for (int i = 0; i < 3; ++i)
			{
				if (reVert.at(t[i]) == invalid)
				{
					reVert.at(t[i]) = next++;
				}
				t[i] = reVert.at(t[i]);
			}
		}
		verts.resize(next);
		for (size_t i = 0; i < m_mesh->vertices.size(); ++i)
		{
			const uint32_t idx = reVert.at(i);
			if (idx == invalid) continue;
			verts.at(idx) = m_mesh->vertices.at(i);
		}
		std::swap(verts, m_mesh->vertices);
	}

	return true;
}
