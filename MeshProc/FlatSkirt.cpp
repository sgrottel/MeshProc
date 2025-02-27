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
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::VertexSelection>("Loop", m_loop);
	AddParamBinding<ParamMode::Out, ParamType::VertexSelection>("NewLoop", m_newLoop);
	AddParamBinding<ParamMode::Out, ParamType::Vec3>("Center", m_center);
	AddParamBinding<ParamMode::Out, ParamType::Vec3>("X2D", m_x2D);
	AddParamBinding<ParamMode::Out, ParamType::Vec3>("Y2D", m_y2D);
	AddParamBinding<ParamMode::Out, ParamType::Vec3>("ZDir", m_zDir);
	AddParamBinding<ParamMode::Out, ParamType::Float>("ZDist", m_zDist);
}

bool FlatSkirt::Invoke()
{
	Log().Detail("Adding skirt to loop");

	std::vector<glm::vec3> v;

	const auto& loop = *m_loop;

	v.resize(loop.size());
	std::transform(loop.begin(), loop.end(), v.begin(), [mesh = this->m_mesh](uint32_t i) { return mesh->vertices[i]; });

	data::Triangle oldTri = m_mesh->triangles[0];
	{
		const uint32_t i0 = loop[0];
		const uint32_t i1 = loop[1];
		for (data::Triangle const& t : m_mesh->triangles)
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

	{
		glm::mat3 covarMat = glm::computeCovarianceMatrix(v.data(), v.size(), m_center);

		glm::vec3 evals;
		glm::mat3 evecs;
		int evcnt = glm::findEigenvaluesSymReal(covarMat, evals, evecs);

		if (evcnt != 3)
		{
			Log().Error("Failed to compute princple vectors for open loop");
			return {};
		}

		glm::sortEigenvalues(evals, evecs);

		m_x2D = glm::normalize(evecs[0]);
		m_y2D = glm::normalize(evecs[1]);

		m_zDir = glm::normalize(evecs[2]);
	}

	{
		glm::vec3 triEdgeCenter = v[0] + v[1];
		triEdgeCenter *= 0.5f;
		glm::vec3 d{ 0.0f, 0.0f, 0.0f };
		for (int i = 0; i < 3; ++i) {
			d += m_mesh->vertices[oldTri[i]] - triEdgeCenter;
		}
		float f = glm::dot(d, m_zDir);
		if (f > 0.0f)
		{
			m_zDir *= -1.0f;
		}
	}

	float maxDist = 0.0f;
	for (size_t i = 0; i < v.size(); ++i)
	{
		const float dist = glm::dot(v[i] - m_center, m_zDir);
		if (dist > maxDist)
		{
			maxDist = dist;
		}
		v[i] -= m_zDir * dist;
	}
	maxDist *= 1.1f;
	m_zDist = maxDist;
	for (size_t i = 0; i < v.size(); ++i)
	{
		v[i] += m_zDir * maxDist;
	}

	m_center += m_zDir * maxDist;

	size_t oldSize = m_mesh->vertices.size();
	m_mesh->vertices.reserve(oldSize + v.size());
	for (size_t i = 0; i < v.size(); ++i)
	{
		m_mesh->vertices.push_back(v[i]);
	}

	size_t oldTriSize = m_mesh->triangles.size();
	m_mesh->triangles.reserve(m_mesh->triangles.size() + v.size() * 2);

	for (size_t i = 0; i < v.size(); ++i)
	{
		uint32_t i0 = loop[i];
		uint32_t i1 = loop[(i + 1) % v.size()];
		uint32_t i2 = static_cast<uint32_t>(oldSize + i);
		uint32_t i3 = static_cast<uint32_t>(oldSize + ((i + 1) % v.size()));

		m_mesh->AddQuad(i0, i1, i2, i3);
	}

	{
		data::Triangle newTri = m_mesh->triangles[oldTriSize];
		glm::uvec2 edge = oldTri.CommonEdge(newTri);
		assert(edge.x != edge.y);
		assert((edge.x == loop[0] || edge.y == loop[0]) && (edge.x == loop[1] || edge.y == loop[1]));

		const bool oriented = oldTri.OrientationMatches(m_mesh->vertices, newTri, edge);
		if (!oriented)
		{
			for (size_t i = oldTriSize; i < m_mesh->triangles.size(); ++i)
			{
				m_mesh->triangles[i].Flip();
			}
		}
	}

	std::shared_ptr<std::vector<uint32_t>> newLoop = std::make_shared<ParamTypeInfo_t<ParamType::VertexSelection>::element_type>();
	newLoop->reserve(m_mesh->vertices.size() - oldSize);
	for (size_t i = oldSize; i < m_mesh->vertices.size(); ++i)
	{
		newLoop->push_back(static_cast<uint32_t>(i));
	}

	Log().Detail("Added %d vertices", static_cast<int>(newLoop->size()));

	m_newLoop = newLoop;

	return true;
}
