#include "ProjectionScarf.h"

#include "utilities/Constrained2DTriangulation.h"
#include "utilities/Vec2Intersections.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <SimpleLog/SimpleLog.hpp>

#include <array>
#include <cassert>
#include <unordered_map>
#include <unordered_set>

using namespace meshproc;
using namespace meshproc::commands;

struct compute::ProjectionScarf::Mesh2D
{
};

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
		Log().Error("Projection is empty");
		return false;
	}
	const auto [x2d, y2d] = m_projection->Make2DCoordSys();

	std::vector<uint32_t> selTris = SelectTriangles();
	Mesh2D mesh2d = ProjectTriangles(selTris, x2d, y2d);

	m_outmesh = std::make_shared<data::Mesh>();

	m_outmesh->vertices.resize(m_mesh->vertices.size());
	std::copy(m_mesh->vertices.begin(), m_mesh->vertices.end(), m_outmesh->vertices.begin());

	m_outmesh->triangles.resize(selTris.size());
	for (size_t i = 0; i < selTris.size(); ++i)
	{
		m_outmesh->triangles.at(i) = m_mesh->triangles.at(selTris.at(i));
	}

	uint32_t i = static_cast<uint32_t>(m_outmesh->vertices.size());
	glm::vec3 o = m_projection->Normal() * m_projection->Dist();
	m_outmesh->vertices.push_back(o);
	m_outmesh->vertices.push_back(o + x2d * 10.0f);
	m_outmesh->vertices.push_back(o + y2d * 10.0f);
	m_outmesh->triangles.push_back({ i, i + 1, i + 2 });

	Log().Error("NOT IMPLEMENTED");
	return false;
}

std::vector<uint32_t> compute::ProjectionScarf::SelectTriangles()
{
	std::vector<uint32_t> sel;
	sel.reserve(m_mesh->triangles.size() / 2);

	for (size_t ti = 0; ti < m_mesh->triangles.size(); ++ti)
	{
		const glm::vec3 n = m_mesh->triangles[ti].CalcNormal(m_mesh->vertices);
		if (glm::dot(n, m_projection->Normal()) > 0.0f)
		{
			sel.push_back(static_cast<uint32_t>(ti));
		}
	}

	return sel;
}

compute::ProjectionScarf::Mesh2D compute::ProjectionScarf::ProjectTriangles(std::vector<uint32_t> const& selTris, glm::vec3 const& x2d, glm::vec3 const& y2d)
{
	Mesh2D mesh2D;

	// TODO: Implement

	return mesh2D;
}