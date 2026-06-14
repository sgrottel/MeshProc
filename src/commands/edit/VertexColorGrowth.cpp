#include "VertexColorGrowth.h"

#include "utilities/ColorCompare.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::edit;

VertexColorGrowth::VertexColorGrowth(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("Color", m_color);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("BlankColor", m_blankColor);
	AddParamBinding<ParamMode::InOut, ParamType::Vec3List>("Colors", m_colors);
	AddParamBinding<ParamMode::Out, ParamType::Float>("CountColored", m_countColored);
}

bool VertexColorGrowth::Invoke()
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

	std::vector<uint8_t> marker;
	marker.resize(m_colors->size());
	std::transform(m_colors->begin(), m_colors->end(), marker.begin(),
		[&](glm::vec3 const& c)
		{
			if (ColorCompare::IsNearlyEqual(c, m_color))
			{
				return 2;	// already set
			}
			if (ColorCompare::IsNearlyEqual(c, m_blankColor))
			{
				return 0;	// growth potential
			}
			return 255;		// blocked by another color
		});

	//const size_t cnt0 = std::count_if(marker.begin(), marker.end(), [](uint8_t m) { return m == 0; });
	//const size_t cnt2 = std::count_if(marker.begin(), marker.end(), [](uint8_t m) { return m == 2; });
	//const size_t cntF = std::count_if(marker.begin(), marker.end(), [](uint8_t m) { return m == 255; });

	for (const auto& t : m_mesh->triangles)
	{
		uint8_t& m0 = marker[t[0]];
		uint8_t& m1 = marker[t[1]];
		uint8_t& m2 = marker[t[2]];

		if ((m0 == 2 || m1 == 2 || m2 == 2)
			&& (m0 == 0 || m1 == 0 || m2 == 0))
		{
			if (m0 == 0) m0 = 1;
			if (m1 == 0) m1 = 1;
			if (m2 == 0) m2 = 1;
		}
	}

	const size_t setNew = std::count_if(marker.begin(), marker.end(), [](uint8_t m) { return m == 1; });
	m_countColored = static_cast<float>(setNew);

	if (setNew > 0)
	{
		const auto firstCol = std::find_if(marker.begin(), marker.end(), [](uint8_t m) { return m == 2; });
		assert(firstCol != marker.end());
		const glm::vec3 col = m_colors->at(std::distance(marker.begin(), firstCol));
		for (size_t i = 0; i < marker.size(); ++i)
		{
			if (marker[i] == 1)
			{
				m_colors->at(i) = col;
			}
		}

		Log().Detail(
			"Color growth of (%u, %u, %u) by %u",
			static_cast<int>(255.0f * m_color.r),
			static_cast<int>(255.0f * m_color.g),
			static_cast<int>(255.0f * m_color.b),
			setNew);
	}
	else
	{
		Log().Detail("Color growth halted for (%f, %f, %f)", m_color.r, m_color.g, m_color.b);
	}

	return true;
}
