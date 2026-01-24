#include "VertexEdgeDistance.h"

#include <SimpleLog/SimpleLog.hpp>

#include <array>
#include <numeric>

using namespace meshproc;
using namespace meshproc::commands;

compute::VertexEdgeDistance::VertexEdgeDistance(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::IndexList>("Selection", m_selection);
	AddParamBinding<ParamMode::Out, ParamType::FloatList>("Distances", m_dists);
}

bool compute::VertexEdgeDistance::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_selection)
	{
		Log().Error("Selection is empty");
		return false;
	}

	m_dists = std::make_shared<std::vector<float>>(m_mesh->vertices.size(), std::numeric_limits<float>::quiet_NaN());

	if (m_selection->size() == 0)
	{
		Log().Warning("Selection is empty");
		std::fill(m_dists->begin(), m_dists->end(), 0.0f);
		return true;
	}

	std::unordered_set<size_t> triFront;
	std::unordered_set<size_t> tris;
	triFront.reserve(m_selection->size()); // good enough for an initial estimate; will limit the required reallocations during initialization
	tris.reserve(m_mesh->triangles.size());

	for (uint32_t i : *m_selection)
	{
		// Initialization values?
		// those could break the assumption of the advancing front and could required re-iteration on value updates.
		m_dists->at(i) = 0.0f;
	}

	for (size_t i = 0; i < m_mesh->triangles.size(); ++i)
	{
		const auto& t = m_mesh->triangles.at(i);
		const float v0 = m_dists->at(t[0]);
		const float v1 = m_dists->at(t[1]);
		const float v2 = m_dists->at(t[2]);
		const bool valid0 = !std::isnan(v0);
		const bool valid1 = !std::isnan(v1);
		const bool valid2 = !std::isnan(v2);
		if (valid0 || valid1 || valid2)
		{
			if (valid0 && valid1 && valid2) continue;
			triFront.insert(i);
		}
		else
		{
			tris.insert(i);
		}
	}

	std::unordered_map<size_t, float> newDists;
	newDists.reserve(m_selection->size());
	while (!triFront.empty())
	{
		newDists.clear();
		for (size_t ti : triFront)
		{
			const auto& t = m_mesh->triangles.at(ti);
			std::array<float, 3> v{ m_dists->at(t[0]), m_dists->at(t[1]), m_dists->at(t[2]) };
			std::array<bool, 3> valid{ !std::isnan(v[0]), !std::isnan(v[1]), !std::isnan(v[2]) };
			for (size_t i = 0; i < 3; ++i)
			{
				if (valid[i]) continue;

				for (size_t j = 0; j < 3; ++j)
				{
					if (i == j) continue;
					if (!valid[j]) continue;

					const float dist = v[j] + glm::distance(m_mesh->vertices.at(t[j]), m_mesh->vertices.at(t[i]));
					if (newDists.contains(t[i]))
					{
						if (newDists.at(t[i]) > dist)
						{
							newDists.at(t[i]) = dist;
						}
					}
					else
					{
						newDists.insert(std::make_pair(t[i], dist));
					}
				}
			}
		}

		for (auto [i, v] : newDists)
		{
			m_dists->at(i) = v;
		}

		triFront.clear();
		for (uint32_t ti : tris)
		{
			const auto& t = m_mesh->triangles.at(ti);
			const float v0 = m_dists->at(t[0]);
			const float v1 = m_dists->at(t[1]);
			const float v2 = m_dists->at(t[2]);
			const bool valid0 = !std::isnan(v0);
			const bool valid1 = !std::isnan(v1);
			const bool valid2 = !std::isnan(v2);
			if (valid0 || valid1 || valid2)
			{
				triFront.insert(ti);
			}
		}

		std::erase_if(tris, [&triFront](size_t i) { return triFront.contains(i); });
	}

	// remaining vertices that have no distance yet, are not in the same connected component
	for (float& d : *m_dists)
	{
		if (std::isnan(d))
		{
			d = -1.0f;
		}
	}

	return true;
}
