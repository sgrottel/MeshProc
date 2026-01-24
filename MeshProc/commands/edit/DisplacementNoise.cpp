#include "DisplacementNoise.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_map>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::edit;

DisplacementNoise::DisplacementNoise(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3List>("Dirs", m_dirs);
	AddParamBinding<ParamMode::In, ParamType::Float>("Min", m_min);
	AddParamBinding<ParamMode::In, ParamType::Float>("Max", m_max);
	AddParamBinding<ParamMode::In, ParamType::UInt32>("Seed", m_seed);
}

bool DisplacementNoise::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_dirs)
	{
		Log().Error("Dirs is empty");
		return false;
	}
	if (m_mesh->vertices.size() != m_dirs->size())
	{
		Log().Error("Number of vectors in Mesh and Dirs is not equal");
		return false;
	}

	std::mt19937 gen(m_seed);
	std::uniform_real_distribution<float> random(
		(std::min)(m_min, m_max),
		(std::max)(m_min, m_max));

	for (size_t i = 0; i < m_dirs->size(); ++i)
	{
		m_mesh->vertices.at(i) += m_dirs->at(i) * random(gen);
	}

	return true;
}
