#include "MorphRotateOutline.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/gtc/type_ptr.hpp>

#pragma warning(push)
#pragma warning(disable: 4267 4996)
#undef max
#undef min
#include <flann/flann.hpp>
#pragma warning(pop)

#include <cassert>
#include <unordered_set>

using namespace meshproc;

fitting::MorphRotateOutline::MorphRotateOutline(const sgrottel::ISimpleLog& log)
	: AbstractCommand(log)
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Mesh>("TargetMesh", m_targetMesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("Center", m_center);
	AddParamBinding<ParamMode::In, ParamType::Float>("FixRadius", m_fixRadius);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("PrimaryAxis", m_primaryAxis);
}

bool fitting::MorphRotateOutline::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (m_mesh->vertices.size() < 3)
	{
		Log().Error("Mesh must contain at least three vertices");
		return false;
	}
	if (!m_targetMesh)
	{
		Log().Error("TargetMesh is empty");
		return false;
	}
	if (m_targetMesh->vertices.size() < 3)
	{
		Log().Error("TargetMesh must contain at least three vertices");
		return false;
	}

	// assert that all vertices of the target mesh are stored continuous, aka can be used by flann without copy
	assert(static_cast<void const*>(m_targetMesh->vertices.data()) == static_cast<void const*>(glm::value_ptr(m_targetMesh->vertices.at(0))));
	assert(std::bit_cast<uint8_t const*>(glm::value_ptr(m_targetMesh->vertices.at(0)))
		+ 3 * sizeof(float)
		== static_cast<void const*>(glm::value_ptr(m_targetMesh->vertices.at(1))));
	assert(std::bit_cast<uint8_t const*>(glm::value_ptr(m_targetMesh->vertices.front()))
		+ (3 * sizeof(float)) * (m_targetMesh->vertices.size() - 1)
		== static_cast<void const*>(glm::value_ptr(m_targetMesh->vertices.back())));

	flann::Matrix<float> targetDataSet(
		glm::value_ptr(m_targetMesh->vertices.front()),
		m_targetMesh->vertices.size(),
		3);
	flann::Index<flann::L2<float>> targetIndex(targetDataSet, flann::KDTreeIndexParams(1));
	targetIndex.buildIndex();

	const glm::vec3 axisY = glm::normalize(m_primaryAxis);
	{
		const float len = glm::length(axisY);
		if (len < 0.999 || len > 1.001)
		{
			Log().Error("PrimaryAxis normalization failed");
			return false;
		}
	}

	std::vector<uint32_t> border = GetMeshBorderVertices();
	const size_t borderSize = border.size();

	// Allocate result buffers
	std::vector<float> query_flat(borderSize * 3);
	flann::Matrix<float> query(query_flat.data(), borderSize, 3);
	for (size_t i = 0; i < borderSize; ++i)
	{
		glm::vec3 const& v = m_mesh->vertices.at(border.at(i));
		query_flat.at(i * 3 + 0) = v.x;
		query_flat.at(i * 3 + 1) = v.y;
		query_flat.at(i * 3 + 2) = v.z;
	}

	std::vector<int> indices_vec(borderSize);
	std::vector<float> dists_vec(borderSize);
	flann::Matrix<int> indices(indices_vec.data(), borderSize, 1);
	flann::Matrix<float> dists(dists_vec.data(), borderSize, 1);

	targetIndex.knnSearch(query, indices, dists, 1, flann::SearchParams(128));

	// TODO: Implement

	for (size_t i = 0; i < borderSize; ++i)
	{
		m_mesh->vertices.at(border.at(i)) = m_targetMesh->vertices.at(indices_vec.at(i));
	}

	return false;
}

std::vector<uint32_t> fitting::MorphRotateOutline::GetMeshBorderVertices() const
{
	std::unordered_set<data::HashableEdge> border;
	for (auto const& t : m_mesh->triangles)
	{
		for (uint32_t i = 0; i < 3; ++i)
		{
			auto edge = t.HashableEdge(i);
			if (!border.insert(edge).second)
			{
				border.erase(edge);
			}
		}
	}

	std::unordered_set<uint32_t> vertices;
	for (auto const& e : border)
	{
		vertices.insert(e.i0);
		vertices.insert(e.i1);
	}

	std::vector<uint32_t> result;
	result.resize(vertices.size());

	std::copy(vertices.begin(), vertices.end(), result.begin());

	return result;
}
