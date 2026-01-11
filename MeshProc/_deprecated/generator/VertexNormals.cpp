#include "VertexNormals.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;

generator::VertexNormals::VertexNormals(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParamBinding<ParamMode::Out, ParamType::ListOfVec3>("Normals", m_normals);
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
}

bool generator::VertexNormals::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}

	m_normals = std::make_shared<std::vector<glm::vec3>>(m_mesh->vertices.size(), glm::vec3{});
	for (auto const& t : m_mesh->triangles)
	{
		const glm::vec3 n = t.CalcNormal(m_mesh->vertices);
		const float d = t.CalcSurface(m_mesh->vertices);
		const glm::vec3 v = n * d;
		for (int i = 0; i < 3; ++i)
		{
			m_normals->at(t[i]) += v;
		}
	}

	for (auto& v : *m_normals)
	{
		if (v == glm::vec3{ 0.0 }) continue;
		v = glm::normalize(v);
	}

	return true;
}
