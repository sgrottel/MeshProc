#include "VertexColorCleanup.h"

#include "utilities/ColorCompare.h"

#include <SimpleLog/SimpleLog.hpp>

#include <numeric>
#include <unordered_set>
#include <vector>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::edit;

namespace
{
	inline const float MHD(const glm::vec3& a, const glm::vec3& b) noexcept
	{
		return std::max(std::max(
			std::abs(a.x - b.x),
			std::abs(a.y - b.y)),
			std::abs(a.z - b.z));
	}
}

VertexColorCleanup::VertexColorCleanup(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::InOut, ParamType::Vec3List>("Colors", m_colors);
	AddParamBinding<ParamMode::In, ParamType::Float>("SmallThreshold", m_smallThreshold);
}

bool VertexColorCleanup::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_colors)
	{
		Log().Error("Colors is empty");
		return false;
	}
	if (m_colors->size() != m_mesh->vertices.size())
	{
		Log().Error("Colors and Mesh are not the same size");
		return false;
	}
	const uint32_t smallSize = static_cast<uint32_t>(std::max<float>(0.0f, m_colors->size() * m_smallThreshold));
	if (smallSize <= 0)
	{
		Log().Error("SmallThreshold is zero");
		return false;
	}

	// remove small connected components of each color,
	// by merging it with the surrounding color component with the largest connecting edge

	std::vector<std::unordered_set<uint32_t>> components;
	std::unordered_set<data::HashableEdge> compEdges;
	std::unordered_set<uint32_t> next;
	{
		const size_t len = m_mesh->vertices.size();
		std::vector<std::unordered_set<uint32_t>> edges;
		edges.resize(len);
		for (const auto& t: m_mesh->triangles)
		{
			for (int i = 0; i < 3; ++i)
			{
				const int j = (i + 1) % 3;
				const uint32_t v0 = t[i];
				const uint32_t v1 = t[j];
				if (ColorCompare::IsNearlyEqual(m_colors->at(v0), m_colors->at(v1)))
				{
					edges.at(v0).insert(v1);
					edges.at(v1).insert(v0);
				}
				else
				{
					compEdges.insert({v0, v1});
				}
			}
		}

		std::unordered_set<uint32_t> free;
		free.reserve(len);
		for (size_t i = 0; i < len; ++i)
		{
			free.insert(static_cast<uint32_t>(i));
		}

		std::unordered_set<uint32_t> comp;
		std::unordered_set<uint32_t> border;

		while (!free.empty())
		{
			uint32_t seed = *free.begin();
			free.erase(seed);
			comp.clear();
			border.clear();
			border.insert(seed);

			while (!border.empty())
			{
				next.clear();

				for (uint32_t i : border)
				{
					for (uint32_t ni : edges.at(i))
					{
						if (free.contains(ni))
						{
							next.insert(ni);
							free.erase(ni);
						}
					}
				}

				comp.insert(border.begin(), border.end());
				std::swap(border, next);
			}

			components.push_back(std::move(comp));
		}
	}

	if (components.empty())
	{
		Log().Warning("No components at all?");
		return true;
	}

	std::sort(components.begin(), components.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });

	std::unordered_map<size_t, uint32_t> neighbors;

	while (components.back().size() <= smallSize)
	{
		std::unordered_set<uint32_t> comp = std::move(components.back());
		components.pop_back();

		next.clear();
		for (const auto& e : compEdges)
		{
			const bool c0 = comp.contains(e.i0);
			const bool c1 = comp.contains(e.i1);
			if (c0 && !c1)
			{
				next.insert(e.i1);
			}
			else if (!c0 && c1)
			{
				next.insert(e.i0);
			}
		}

		neighbors.clear();
		for (uint32_t ni : next)
		{
			for (size_t i = 0; i < components.size(); ++i)
			{
				if (components[i].contains(ni))
				{
					if (!neighbors.contains(i))
					{
						neighbors.insert({i, 0});
					}
					neighbors[i]++;
				}
			}
		}

		if (neighbors.empty()) continue;
		size_t mergeInto = neighbors.begin()->first;
		if (neighbors.size() > 1)
		{
			uint32_t cnt = neighbors.begin()->second;
			for (const auto& p : neighbors)
			{
				if (p.second > cnt)
				{
					mergeInto = p.first;
					cnt = p.second;
				}
			}
		}

		glm::vec3 col = m_colors->at(*components.at(mergeInto).begin());
		for (uint32_t i : comp)
		{
			m_colors->at(i) = col;
		}
		components.at(mergeInto).insert(comp.begin(), comp.end());
		std::sort(components.begin(), components.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });
	}

	assert(
		std::accumulate(components.begin(), components.end(), static_cast<size_t>(0), [](size_t cnt, const auto& c) { return cnt + c.size(); })
		== m_mesh->vertices.size());

	Log().Write("Filteres to %d components", components.size());

	return true;
}
