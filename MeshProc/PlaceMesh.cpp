#include "PlaceMesh.h"

using namespace meshproc;

PlaceMesh::PlaceMesh(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Scene>("Scene", m_scene);
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Mat4>("Mat", m_mat);
}

bool PlaceMesh::Invoke()
{
	if (!m_scene)
	{
		m_scene = std::make_shared<data::Scene>();
	}
	m_scene->m_meshes.push_back({ m_mesh, m_mat });
	return true;
}
