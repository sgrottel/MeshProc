#include "ProjectionScarf.h"

#include <SimpleLog/SimpleLog.hpp>

using namespace meshproc;
using namespace meshproc::commands;

compute::ProjectionScarf::ProjectionScarf(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::HalfSpace>("Projection", m_projection);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("OutMesh", m_outmesh);
}

bool compute::ProjectionScarf::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_projection)
	{
		Log().Error("Mesh is empty");
		return false;
	}

	// TODO: Implement

	// 1. select triangles (dot(tri.normal, proj.normal) > 0) & at least one vertex in positive half space
	// cut triangles as necessary

	// 2. created projected edges

	// 3. further cut triangles by projected edges -> be aware of t-vertices in the projected edge

	// 4. created projected vertices

	// 5. scarf geometry

	// 6. scarf base hole close polygon

	m_outmesh = std::make_shared<data::Mesh>();
	m_outmesh->vertices.resize(m_mesh->vertices.size());
	std::transform(
		m_mesh->vertices.begin(),
		m_mesh->vertices.end(),
		m_outmesh->vertices.begin(),
		[&](glm::vec3 const& v)
		{
			return v - m_projection->Normal() * m_projection->Dist(v);
		});


	Log().Error("NOT IMPLEMENTED");
	return false;
}
