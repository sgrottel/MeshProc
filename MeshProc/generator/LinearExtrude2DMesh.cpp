#include "LinearExtrude2DMesh.h"

#include "data/Mesh.h"
#include "data/Shape2D.h"

using namespace meshproc;
using namespace meshproc::generator;

LinearExtrude2DMesh::LinearExtrude2DMesh(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Float>("MinZ", m_minZ);
	AddParamBinding<ParamMode::In, ParamType::Float>("MaxZ", m_maxZ);
	AddParamBinding<ParamMode::In, ParamType::Shape2D>("Shape2D", m_shape2D);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("Mesh", m_mesh);
}

bool LinearExtrude2DMesh::Invoke()
{
	m_mesh = std::make_shared<data::Mesh>();

	const float minZ = std::min(m_minZ, m_maxZ);
	const float maxZ = std::max(m_minZ, m_maxZ);

	// TODO: Implement

	return false;
}
