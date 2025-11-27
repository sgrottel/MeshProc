#include "RotateExtrude2DMesh.h"

#include "data/Mesh.h"
#include "data/Shape2D.h"

#include <glm/glm.hpp>

#include <SimpleLog/SimpleLog.hpp>

#include <numbers>

using namespace meshproc;
using namespace meshproc::generator;

RotateExtrude2DMesh::RotateExtrude2DMesh(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Float>("MinAngle", m_minAngle);
	AddParamBinding<ParamMode::In, ParamType::Float>("MaxAngle", m_maxAngle);
	AddParamBinding<ParamMode::In, ParamType::UInt32>("Steps", m_steps);
	AddParamBinding<ParamMode::In, ParamType::Shape2D>("Shape2D", m_shape2D);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("Mesh", m_mesh);
	//AddParamBinding<ParamMode::Out, ParamType::Indices>("StartLoop", m_startLoop);
	//AddParamBinding<ParamMode::Out, ParamType::Indices>("EndLoop", m_endLoop);
}

bool RotateExtrude2DMesh::Invoke()
{
	if (!m_shape2D)
	{
		Log().Error("Shape2D not set");
		return false;
	}
	size_t shapeTotalLen = 0;
	for (auto const& l : m_shape2D->loops)
	{
		const size_t ls = l.second.size();
		if (ls >= 3)
		{
			shapeTotalLen += ls;
		}
	}
	if (shapeTotalLen == 0)
	{
		Log().Error("Shape2D does not contain valid loops (with three or more vertices)");
		return false;
	}

	if (m_steps == 0)
	{
		Log().Error("Steps must be larger than zero");
		return false;
	}

	auto fixAngle = [](float a)
		{
			if (a < -360.0f)
			{
				return std::fmod(a, 360.0f);
			}
			else if (a > 360.0f)
			{
				return std::fmod(a, 360.0f);
			}
			return a;
		};
	const float a1 = fixAngle(m_minAngle);
	const float a2 = fixAngle(m_maxAngle);
	float aMin = (std::min)(a1, a2);
	float aMax = (std::max)(a1, a2);
	bool isClosed = false;
	if (aMax - aMin >= 360.0f)
	{
		isClosed = true;
		aMax = aMin + 360.0f;
	}

	const uint32_t minSteps = static_cast<uint32_t>(std::ceil((aMax - aMin) / 120));
	if (m_steps < minSteps)
	{
		Log().Error("For a rotation of %f deg. you need to use at least %d steps; specified to use %d steps", aMax - aMin, minSteps, m_steps);
		return false;
	}

	m_mesh = std::make_shared<data::Mesh>();

	const uint32_t vertSteps = m_steps + (isClosed ? 0 : 1);
	m_mesh->vertices.reserve(shapeTotalLen * vertSteps);

	m_mesh->triangles.reserve(shapeTotalLen * (m_steps + 1));

	for (size_t step = 0; step < vertSteps; ++step)
	{
		const float a = (aMin + (aMax - aMin) * static_cast<float>(step) / static_cast<float>(m_steps)) / 180.0f * static_cast<float>(std::numbers::pi);
		const float cos = std::cos(a);
		const float sin = std::sin(a);

		for (auto const& l : m_shape2D->loops)
		{
			const size_t ls = l.second.size();
			if (ls < 3) continue;
			for (glm::vec2 const& v : l.second)
			{
				m_mesh->vertices.push_back(glm::vec3{ v.x * cos, v.x * sin, v.y });
			}
		}
	}

	size_t vIdx = 0;
	for (auto const& l : m_shape2D->loops)
	{
		const size_t ls = l.second.size();
		if (ls < 3) continue;
		for (size_t step = 0; step < m_steps; ++step)
		{
			size_t vLIdx = 0;
			for (glm::vec2 const& v : l.second)
			{
				const size_t i1 = vLIdx;
				const size_t i2 = (vLIdx + 1) % ls;
				size_t nextStep = step + 1;
				if (nextStep * shapeTotalLen >= m_mesh->vertices.size())
				{
					nextStep = 0;
				}
				vLIdx++;
				m_mesh->AddQuad(
					static_cast<uint32_t>(shapeTotalLen * step + vIdx + i1),
					static_cast<uint32_t>(shapeTotalLen * step + vIdx + i2),
					static_cast<uint32_t>(shapeTotalLen * nextStep + vIdx + i1),
					static_cast<uint32_t>(shapeTotalLen * nextStep + vIdx + i2));
			}
		}
		vIdx += ls;
	}

	// TODO: triangle orientation?
	// TODO: caps?
	// TODO: StartLoop and EndLoop?

	return true;
}
