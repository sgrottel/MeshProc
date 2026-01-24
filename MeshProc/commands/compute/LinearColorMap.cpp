#include "LinearColorMap.h"

#include <SimpleLog/SimpleLog.hpp>

#include <functional>

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

	Log().Detail("Input value range: %f .. %f", minVal, maxVal);

	std::function<glm::vec3(float)> color;


	if (minVal < 0.0f && maxVal > 0.0f)
	{
		color = [&](float f)
			{
				if (f >= 0.0f)
				{
					const float b = f / maxVal;
					const float a = 1.0f - b;
					return glm::normalize(
						glm::normalize(glm::vec3{ 1.0f, 1.0f, 1.0f }) * a
						+ glm::vec3{ 0.0f, 0.0f, 1.0f } * b);
				}
				else
				{
					const float b = -f / -minVal;
					const float a = 1.0f - b;
					return glm::normalize(
						glm::normalize(glm::vec3{ 1.0f, 1.0f, 1.0f }) * a
						+ glm::vec3{ 1.0f, 0.0f, 0.0f } *b);
				}
			};
	}
	else
	{
		auto rescale = [&](float v)
			{
				return (v - minVal) / (maxVal - minVal);
			};
		auto colord = [&](float v)
			{
				return glm::normalize(glm::vec3{ 1.0f - v, v, 0.0f });
			};
		color = [=](float f) { return colord(rescale(f)); };
	}

	std::transform(
		m_scalars->begin(),
		m_scalars->end(),
		m_colors->begin(),
		color);

	return true;
}
