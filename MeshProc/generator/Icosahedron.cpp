#include "Icosahedron.h"

#include <cmath>

using namespace meshproc;

generator::Icosahedron::Icosahedron(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParam("Mesh", Mesh);
}

bool generator::Icosahedron::Invoke()
{
	std::shared_ptr<data::Mesh> m = std::make_shared<data::Mesh>();

	m->vertices.reserve(12);
	float a = 2.0f;
	float c = (1.0f + std::sqrtf(5.0f)) * 0.5f * a;

	for (int s1 = -1; s1 <= 1; s1 += 2)
	{
		for (int s2 = -1; s2 <= 1; s2 += 2)
		{
			m->vertices.push_back(glm::vec3{ 0.0f, a * 0.5f * s1, c * 0.5f * s2 });
			m->vertices.push_back(glm::vec3{ c * 0.5f * s2, 0.0f, a * 0.5f * s1 });
			m->vertices.push_back(glm::vec3{ a * 0.5f * s1, c * 0.5f * s2, 0.0f });
		}
	}

	for (auto& v : m->vertices)
	{
		v = glm::normalize(v);
	}

	m->triangles.reserve(20);

	m->triangles.push_back({ 0, 1, 6 });
	m->triangles.push_back({ 0, 6, 4 });
	m->triangles.push_back({ 3, 9, 7 });
	m->triangles.push_back({ 3, 10, 9 });
	m->triangles.push_back({ 1, 2, 7 });
	m->triangles.push_back({ 1, 7, 5 });
	m->triangles.push_back({ 4, 10, 8 });
	m->triangles.push_back({ 4, 11, 10 });
	m->triangles.push_back({ 0, 8, 2 });
	m->triangles.push_back({ 3, 2, 8 });
	m->triangles.push_back({ 6, 5, 11 });
	m->triangles.push_back({ 9, 11, 5 });
	m->triangles.push_back({ 0, 2, 1 });
	m->triangles.push_back({ 0, 4, 8 });
	m->triangles.push_back({ 3, 7, 2 });
	m->triangles.push_back({ 3, 8, 10 });
	m->triangles.push_back({ 6, 1, 5 });
	m->triangles.push_back({ 6, 11, 4 });
	m->triangles.push_back({ 9, 5, 7 });
	m->triangles.push_back({ 9, 10, 11 });

	Mesh.Put() = m;
	return true;
}
