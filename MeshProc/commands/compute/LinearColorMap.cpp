#include "LinearColorMap.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;

compute::LinearColorMap::LinearColorMap(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParamBinding<ParamMode::In, ParamType::FloatList>("Scalars", m_scalars);
	AddParamBinding<ParamMode::Out, ParamType::Vec3List>("Colors", m_colors);
}

bool compute::LinearColorMap::Invoke()
{
	if (!m_scalars)
	{
		Log().Error("Scalars is empty");
		return false;
	}

	m_colors = std::make_shared<std::vector<glm::vec3>>(m_scalars->size(), glm::vec3{});

	const auto [minvalIt, maxvalIt] = std::minmax_element(m_scalars->begin(), m_scalars->end());
	const float minVal = *minvalIt;
	const float maxVal = (*maxvalIt <= minVal) ? (minVal + 1.0f) : (*maxvalIt);

	auto rescale = [&](float v)
		{
			return (v - minVal) / (maxVal - minVal);
		};
	auto color = [&](float v)
		{
			return glm::normalize(glm::vec3{ 1.0f - v, v, 0.0f });
		};

	std::transform(
		m_scalars->begin(),
		m_scalars->end(),
		m_colors->begin(),
		[&](float v)
		{
			return color(rescale(v));
		});

	return true;
}
