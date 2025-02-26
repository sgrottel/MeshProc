#include "FlatSkirt.h"

#include <SimpleLog/SimpleLog.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/pca.hpp>

#include <algorithm>
#include <numeric>

using namespace meshproc;

FlatSkirt::FlatSkirt(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParam("Mesh", Mesh);
	AddParam("Loop", Loop);
	AddParam("NewLoop", NewLoop);
	AddParam("Center", Center);
	AddParam("X2D", X2D);
	AddParam("Y2D", Y2D);
	AddParam("ZDir", ZDir);
	AddParam("ZDist", ZDist);
}

bool FlatSkirt::Invoke()
{
	Log().Detail("Adding skirt to loop");

	std::vector<glm::vec3> v;

	auto const& loop = Loop.Get();
	auto& mesh = Mesh.Put();

	v.resize(loop.size());
	std::transform(loop.begin(), loop.end(), v.begin(), [&mesh](uint32_t i) { return mesh->vertices[i]; });

	data::Triangle oldTri = mesh->triangles[0];
	{
		const uint32_t i0 = loop[0];
		const uint32_t i1 = loop[1];
		for (data::Triangle const& t : mesh->triangles)
		{
			if (t.HasIndex(i0) && t.HasIndex(i1))
			{
				oldTri = t;
				break;
			}
		}
	}

	auto& center = Center.Put();
	auto& zDir = ZDir.Put();

	center = std::reduce(v.begin(), v.end());
	center /= v.size();

	{
		glm::mat3 covarMat = glm::computeCovarianceMatrix(v.data(), v.size(), center);

		glm::vec3 evals;
		glm::mat3 evecs;
		int evcnt = glm::findEigenvaluesSymReal(covarMat, evals, evecs);

		if (evcnt != 3)
		{
			Log().Error("Failed to compute princple vectors for open loop");
			return {};
		}

		glm::sortEigenvalues(evals, evecs);

		X2D.Put() = glm::normalize(evecs[0]);
		Y2D.Put() = glm::normalize(evecs[1]);

		zDir = glm::normalize(evecs[2]);
	}

	{
		glm::vec3 triEdgeCenter = v[0] + v[1];
		triEdgeCenter *= 0.5f;
		glm::vec3 d{ 0.0f, 0.0f, 0.0f };
		for (int i = 0; i < 3; ++i) {
			d += mesh->vertices[oldTri[i]] - triEdgeCenter;
		}
		float f = glm::dot(d, zDir);
		if (f > 0.0f)
		{
			zDir *= -1.0f;
		}
	}

	float maxDist = 0.0f;
	for (size_t i = 0; i < v.size(); ++i)
	{
		const float dist = glm::dot(v[i] - center, zDir);
		if (dist > maxDist)
		{
			maxDist = dist;
		}
		v[i] -= zDir * dist;
	}
	maxDist *= 1.1f;
	ZDist.Put() = maxDist;
	for (size_t i = 0; i < v.size(); ++i)
	{
		v[i] += zDir * maxDist;
	}

	center += zDir * maxDist;

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
		data::Triangle newTri = mesh->triangles[oldTriSize];
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

	Log().Detail("Added %d vertices", static_cast<int>(newLoop.size()));

	std::swap(NewLoop.Put(), newLoop);

	return true;
}
