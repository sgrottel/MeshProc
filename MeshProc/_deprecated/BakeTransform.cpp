#include "BakeTransform.h"

#include <glm/glm.hpp>

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

BakeTransform::BakeTransform(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Mat4>("Matrix", m_matrix);
}

bool BakeTransform::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh not set");
		return false;
	}

	for (auto& v : m_mesh->vertices)
	{
		glm::vec4 hv = m_matrix * glm::vec4{ v, 1 };
		v.x = hv.x / hv.w;
		v.y = hv.y / hv.w;
		v.z = hv.z / hv.w;
	}

	return true;
}
