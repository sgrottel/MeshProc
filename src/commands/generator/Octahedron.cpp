#include "Octahedron.h"

#include <cmath>

using namespace meshproc;
using namespace meshproc::commands;

generator::Octahedron::Octahedron(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("Mesh", m_mesh);
}

bool generator::Octahedron::Invoke()
{
	std::shared_ptr<data::Mesh> m = std::make_shared<data::Mesh>();

	m->vertices.reserve(6);
	m->vertices.push_back({ -1, 0, 0 });
	m->vertices.push_back({ 1, 0, 0 });
	m->vertices.push_back({ 0, -1, 0 });
	m->vertices.push_back({ 0, 1, 0 });
	m->vertices.push_back({ 0, 0, -1 });
	m->vertices.push_back({ 0, 0, 1 });

	m->triangles.reserve(8);

	m->triangles.push_back({ 0, 4, 2 });
	m->triangles.push_back({ 1, 2, 4 });
	m->triangles.push_back({ 0, 3, 4 });
	m->triangles.push_back({ 1, 4, 3 });
	m->triangles.push_back({ 0, 2, 5 });
	m->triangles.push_back({ 1, 5, 2 });
	m->triangles.push_back({ 0, 5, 3 });
	m->triangles.push_back({ 1, 3, 5 });

	m_mesh = m;
	return true;
}
