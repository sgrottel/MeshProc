#include "CubeGenerator.h"

std::shared_ptr<Mesh> CubeGenerator::Create(float sizeX, float sizeY, float sizeZ)
{
	std::shared_ptr<Mesh> m = std::make_shared<Mesh>();

	m->vertices.reserve(8);
	m->vertices.push_back(glm::vec3{ 0.0f, 0.0f, 0.0f });	// 0
	m->vertices.push_back(glm::vec3{ sizeX, 0.0f, 0.0f });	// 1
	m->vertices.push_back(glm::vec3{ 0.0f, sizeY, 0.0f });	// 2
	m->vertices.push_back(glm::vec3{ sizeX, sizeY, 0.0f });	// 3
	m->vertices.push_back(glm::vec3{ 0.0f, 0.0f, sizeZ });	// 4
	m->vertices.push_back(glm::vec3{ sizeX, 0.0f, sizeZ });	// 5
	m->vertices.push_back(glm::vec3{ 0.0f, sizeY, sizeZ });	// 6
	m->vertices.push_back(glm::vec3{ sizeX, sizeY, sizeZ });// 7

	m->triangles.reserve(12);
	auto quadFace = [&](unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4)
		{
			m->triangles.push_back(Triangle{ i1, i2, i3 });
			m->triangles.push_back(Triangle{ i3, i2, i4 });
		};
	quadFace(0, 2, 1, 3);
	quadFace(0, 1, 4, 5);
	quadFace(0, 4, 2, 6);
	quadFace(3, 2, 7, 6);
	quadFace(4, 5, 6, 7);
	quadFace(1, 3, 5, 7);

	return m;
}
