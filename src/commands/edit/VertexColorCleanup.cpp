#include "VertexColorCleanup.h"

#include "utilities/ColorCompare.h"

#include <SimpleLog/SimpleLog.hpp>

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

	// remove small connected components of each color,
	// by merging it with the surrounding color component with the largest connecting edge

	using IndexSet = std::unordered_set<uint32_t>;

	IndexSet rest;
	rest.reserve(m_colors->size());
	for (uint32_t i = 0; i < static_cast<uint32_t>(m_colors->size()); ++i)
	{
		rest.insert(i);
	}

	IndexSet border;
	IndexSet set;
	glm::vec3 col;
	std::vector<IndexSet> sets;

	while (!rest.empty())
	{
		set.clear();
		border.clear();
		{
			uint32_t seed = *rest.begin();
			border.insert(seed);
			rest.erase(seed);
			col = m_colors->at(seed);
		}

		while (!border.empty())
		{
			
		}


	}

	// TODO: Implement

	return false;
}
