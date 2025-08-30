#include "MorphRotateOutline.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

fitting::MorphRotateOutline::MorphRotateOutline(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Mesh>("TargetMesh", m_targetMesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("Center", m_center);
	AddParamBinding<ParamMode::In, ParamType::Float>("FixRadius", m_fixRadius);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("PrimaryAxis", m_primaryAxis);
}

bool fitting::MorphRotateOutline::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_targetMesh)
	{
		Log().Error("TargetMesh is empty");
		return false;
	}

	const glm::vec3 axisY = glm::normalize(m_primaryAxis);
	{
		const float len = glm::length(axisY);
		if (len < 0.999 || len > 1.001)
		{
			Log().Error("PrimaryAxis normalization failed");
			return false;
		}
	}

	// TODO: Implement

	return false;
}
