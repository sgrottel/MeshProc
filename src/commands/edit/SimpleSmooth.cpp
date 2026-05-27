#include "SimpleSmooth.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>
#include <vector>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::edit;

SimpleSmooth::SimpleSmooth(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Float>("Strength", m_strength);
}

bool SimpleSmooth::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (m_strength <= 0.0f)
	{
		Log().Warning("Smooth strength zero or smaller deactivates command");
		return true;
	}
	if (m_strength > 1.0f)
	{
		Log().Warning("Smooth strength clamped to 1.0f");
	}
	const float strength = glm::clamp(m_strength, 0.0f, 1.0f);

	std::vector<glm::vec4> midvals{ m_mesh->vertices.size(), glm::vec4{ 0.0f } };

	for (const auto& t : m_mesh->triangles)
	{
		glm::vec3 tm{ 0.0f };
		for (uint32_t j = 0; j < 3; ++j)
		{
			tm += m_mesh->vertices.at(t[j]);
		}
		tm /= 3.0f;
		for (uint32_t j = 0; j < 3; ++j)
		{
			midvals.at(t[j]) += glm::vec4{ tm, 1.0f };
		}
	}

	for (size_t i = 0; i < m_mesh->vertices.size(); ++i)
	{
		glm::vec3& v = m_mesh->vertices.at(i);
		const glm::vec4& tm = midvals.at(i);
		v = glm::mix(v, glm::vec3{ tm.x / tm.w, tm.y / tm.w, tm.z / tm.w }, strength);
	}

	return true;
}
