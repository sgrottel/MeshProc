#include "MeshSmooth.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/gtc/type_ptr.hpp>

#pragma warning(push)
#pragma warning(disable: 4267 4996)
#undef max
#undef min
#include <flann/flann.hpp>
#pragma warning(pop)

#include <cassert>

using namespace meshproc;

MeshSmooth::MeshSmooth(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Indices>("SelectedVertices", m_vertSel);
	AddParamBinding<ParamMode::In, ParamType::Float>("Radius", m_rad);
}

bool MeshSmooth::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("InputMesh is empty");
		return false;
	}
	if (!m_vertSel)
	{
		// todo: future feature -- without selection, select all triangles
		Log().Error("SelectedVertices is empty");
		return false;
	}

	// assert that all vertices of the target mesh are stored continuous, aka can be used by flann without copy
	assert(static_cast<void const*>(m_mesh->vertices.data()) == static_cast<void const*>(glm::value_ptr(m_mesh->vertices.at(0))));
	assert(std::bit_cast<uint8_t const*>(glm::value_ptr(m_mesh->vertices.at(0)))
		+ 3 * sizeof(float)
		== static_cast<void const*>(glm::value_ptr(m_mesh->vertices.at(1))));
	assert(std::bit_cast<uint8_t const*>(glm::value_ptr(m_mesh->vertices.front()))
		+ (3 * sizeof(float)) * (m_mesh->vertices.size() - 1)
		== static_cast<void const*>(glm::value_ptr(m_mesh->vertices.back())));

	flann::Matrix<float> mesh{
		glm::value_ptr(m_mesh->vertices.front()),
		m_mesh->vertices.size(),
		3
		};
	flann::Index<flann::L2<float>> meshIndex{ mesh, flann::KDTreeIndexParams(1) };
	meshIndex.buildIndex();

	// prepare query
	std::vector<float> query_flat(m_vertSel->size() * 3);
	flann::Matrix<float> query(query_flat.data(), m_vertSel->size(), 3);
	for (size_t i = 0; i < m_vertSel->size(); ++i)
	{
		glm::vec3 const& v1 = m_mesh->vertices.at(m_vertSel->at(i));
		query_flat.at(i * 3 + 0) = v1.x;
		query_flat.at(i * 3 + 1) = v1.y;
		query_flat.at(i * 3 + 2) = v1.z;
	}

	// results
	std::vector<std::vector<size_t>> indices;
	std::vector<std::vector<float>> distances;

	meshIndex.radiusSearch(query, indices, distances, m_rad, flann::SearchParams(128));

	// todo implement

	std::vector<glm::vec4> newPos;
	newPos.resize(m_vertSel->size(), glm::vec4{ 0 });
	for (size_t i = 0; i < m_vertSel->size(); i++)
	{
		for (size_t j : indices.at(i))
		{
			newPos[i] += glm::vec4{ m_mesh->vertices.at(j), 1.0f };
		}
	}

	for (size_t i = 0; i < m_vertSel->size(); i++)
	{
		m_mesh->vertices.at(m_vertSel->at(i)) = glm::vec3(newPos[i]) / newPos[i].w;
	}

	return true;
}
