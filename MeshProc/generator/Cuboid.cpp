#include "Cuboid.h"

using namespace meshproc;

generator::Cuboid::Cuboid(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParamBinding<ParamMode::In, ParamType::Float>("SizeX", m_sizeX);
	AddParamBinding<ParamMode::In, ParamType::Float>("SizeY", m_sizeY);
	AddParamBinding<ParamMode::In, ParamType::Float>("SizeZ", m_sizeZ);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("Mesh", m_mesh);
}

bool generator::Cuboid::Invoke()
{
	std::shared_ptr<data::Mesh> m = std::make_shared<data::Mesh>();

	m->vertices.reserve(8);
	m->vertices.push_back(glm::vec3{ 0.0f, 0.0f, 0.0f });			// 0
	m->vertices.push_back(glm::vec3{ m_sizeX, 0.0f, 0.0f });		// 1
	m->vertices.push_back(glm::vec3{ 0.0f, m_sizeY, 0.0f });		// 2
	m->vertices.push_back(glm::vec3{ m_sizeX, m_sizeY, 0.0f });		// 3
	m->vertices.push_back(glm::vec3{ 0.0f, 0.0f, m_sizeZ });		// 4
	m->vertices.push_back(glm::vec3{ m_sizeX, 0.0f, m_sizeZ });		// 5
	m->vertices.push_back(glm::vec3{ 0.0f, m_sizeY, m_sizeZ });		// 6
	m->vertices.push_back(glm::vec3{ m_sizeX, m_sizeY, m_sizeZ });	// 7

	m->triangles.reserve(12);

	m->AddQuad(0, 2, 1, 3);
	m->AddQuad(0, 1, 4, 5);
	m->AddQuad(0, 4, 2, 6);
	m->AddQuad(3, 2, 7, 6);
	m->AddQuad(4, 5, 6, 7);
	m->AddQuad(1, 3, 5, 7);

	m_mesh = m;
	return true;
}
