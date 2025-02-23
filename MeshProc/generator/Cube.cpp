#include "Cube.h"

using namespace meshproc;

generator::Cube::Cube(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParam("SizeX", SizeX);
	AddParam("SizeY", SizeY);
	AddParam("SizeZ", SizeZ);
	AddParam("Mesh", Mesh);
}

bool generator::Cube::Invoke()
{
	std::shared_ptr<data::Mesh> m = std::make_shared<data::Mesh>();

	m->vertices.reserve(8);
	m->vertices.push_back(glm::vec3{ 0.0f, 0.0f, 0.0f });						// 0
	m->vertices.push_back(glm::vec3{ SizeX.Get(), 0.0f, 0.0f });				// 1
	m->vertices.push_back(glm::vec3{ 0.0f, SizeY.Get(), 0.0f });				// 2
	m->vertices.push_back(glm::vec3{ SizeX.Get(), SizeY.Get(), 0.0f });			// 3
	m->vertices.push_back(glm::vec3{ 0.0f, 0.0f, SizeZ.Get() });				// 4
	m->vertices.push_back(glm::vec3{ SizeX.Get(), 0.0f, SizeZ.Get() });			// 5
	m->vertices.push_back(glm::vec3{ 0.0f, SizeY.Get(), SizeZ.Get() });			// 6
	m->vertices.push_back(glm::vec3{ SizeX.Get(), SizeY.Get(), SizeZ.Get() });	// 7

	m->triangles.reserve(12);

	m->AddQuad(0, 2, 1, 3);
	m->AddQuad(0, 1, 4, 5);
	m->AddQuad(0, 4, 2, 6);
	m->AddQuad(3, 2, 7, 6);
	m->AddQuad(4, 5, 6, 7);
	m->AddQuad(1, 3, 5, 7);

	Mesh.Put() = m;
	return true;
}
