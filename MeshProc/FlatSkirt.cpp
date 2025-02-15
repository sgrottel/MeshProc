#include "FlatSkirt.h"

#include <SimpleLog/SimpleLog.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/pca.hpp>

#include <algorithm>
#include <numeric>

FlatSkirt::FlatSkirt(sgrottel::ISimpleLog& log)
	:m_log{ log }
{
}

std::vector<uint32_t> FlatSkirt::AddSkirt(std::shared_ptr<Mesh>& mesh, std::vector<uint32_t> const& loop)
{
	std::vector<glm::vec3> v;
	v.resize(loop.size());
	std::transform(loop.begin(), loop.end(), v.begin(), [&mesh](uint32_t i) { return mesh->vertices[i]; });

	Triangle oldTri = mesh->triangles[0];
	{
		const uint32_t i0 = loop[0];
		const uint32_t i1 = loop[1];
		for (Triangle const& t : mesh->triangles)
		{
			if (t.HasIndex(i0) && t.HasIndex(i1))
			{
				oldTri = t;
				break;
			}
		}
	}

	m_center = std::reduce(v.begin(), v.end());
	m_center /= v.size();

	glm::vec3 dir;
	{
		glm::mat3 covarMat = glm::computeCovarianceMatrix(v.data(), v.size(), m_center);

		glm::vec3 evals;
		glm::mat3 evecs;
		int evcnt = glm::findEigenvaluesSymReal(covarMat, evals, evecs);

		if (evcnt != 3)
		{
			m_log.Error("Failed to compute princple vectors for open loop");
			return {};
		}

		glm::sortEigenvalues(evals, evecs);

		m_x2d = glm::normalize(evecs[0]);
		m_y2d = glm::normalize(evecs[1]);

		dir = glm::normalize(evecs[2]);
	}

	{
		glm::vec3 triEdgeCenter = v[0] + v[1];
		triEdgeCenter *= 0.5f;
		glm::vec3 d{ 0.0f, 0.0f, 0.0f };
		for (int i = 0; i < 3; ++i) {
			d += mesh->vertices[oldTri[i]] - triEdgeCenter;
		}
		float f = glm::dot(d, dir);
		if (f > 0.0f)
		{
			dir *= -1.0f;
		}
	}

	float maxDist = 0.0f;
	for (size_t i = 0; i < v.size(); ++i)
	{
		const float dist = glm::dot(v[i] - m_center, dir);
		if (dist > maxDist)
		{
			maxDist = dist;
		}
		v[i] -= dir * dist;
	}
	maxDist *= 1.1f;
	for (size_t i = 0; i < v.size(); ++i)
	{
		v[i] += dir * maxDist;
	}

	m_center += dir * maxDist;

	size_t oldSize = mesh->vertices.size();
	mesh->vertices.reserve(oldSize + v.size());
	for (size_t i = 0; i < v.size(); ++i)
	{
		mesh->vertices.push_back(v[i]);
	}

	size_t oldTriSize = mesh->triangles.size();
	mesh->triangles.reserve(mesh->triangles.size() + v.size() * 2);

	for (size_t i = 0; i < v.size(); ++i)
	{
		uint32_t i0 = loop[i];
		uint32_t i1 = loop[(i + 1) % v.size()];
		uint32_t i2 = static_cast<uint32_t>(oldSize + i);
		uint32_t i3 = static_cast<uint32_t>(oldSize + ((i + 1) % v.size()));

		mesh->AddQuad(i0, i1, i2, i3);
	}

	{
		Triangle newTri = mesh->triangles[oldTriSize];
		glm::uvec2 edge = oldTri.CommonEdge(newTri);
		assert(edge.x != edge.y);
		assert((edge.x == loop[0] || edge.y == loop[0]) && (edge.x == loop[1] || edge.y == loop[1]));

		const bool oriented = oldTri.OrientationMatches(mesh->vertices, newTri, edge);
		if (!oriented)
		{
			for (size_t i = oldTriSize; i < mesh->triangles.size(); ++i)
			{
				mesh->triangles[i].Flip();
			}
		}
	}

	std::vector<uint32_t> newLoop;
	newLoop.reserve(mesh->vertices.size() - oldSize);
	for (size_t i = oldSize; i < mesh->vertices.size(); ++i)
	{
		newLoop.push_back(static_cast<uint32_t>(i));
	}

	return newLoop;
}
