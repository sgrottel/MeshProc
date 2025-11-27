#include "MeasureBoundingBox.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

MeasureBoundingBox::MeasureBoundingBox(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::Out, ParamType::Vec3>("Min", m_min);
	AddParamBinding<ParamMode::Out, ParamType::Vec3>("Max", m_max);
}

bool MeasureBoundingBox::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh not set");
		return false;
	}
	if (m_mesh->vertices.size() <= 0)
	{
		Log().Error("Mesh is empty");
		return false;
	}

	m_max = m_min = m_mesh->vertices.front();

	for (glm::vec3 const& v : m_mesh->vertices)
	{
		if (m_min.x > v.x) m_min.x = v.x;
		if (m_min.y > v.y) m_min.y = v.y;
		if (m_min.z > v.z) m_min.z = v.z;
		if (m_max.x < v.x) m_max.x = v.x;
		if (m_max.y < v.y) m_max.y = v.y;
		if (m_max.z < v.z) m_max.z = v.z;
	}

	Log().Detail("BBox: [%f, %f, %f] [%f, %f, %f]",
		m_min.x, m_min.y, m_min.z,
		m_max.x, m_max.y, m_max.z);

	return true;
}
