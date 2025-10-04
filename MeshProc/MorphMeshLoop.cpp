#include "MorphMeshLoop.h"

#include <SimpleLog/SimpleLog.hpp>

#include <unordered_set>
#include <unordered_map>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using namespace meshproc;

MorphMeshLoop::MorphMeshLoop(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::InOut, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Indices>("EdgeList", m_edgeList);
	AddParamBinding<ParamMode::In, ParamType::Float>("BlendArea", m_blendArea);
	AddParamBinding<ParamMode::In, ParamType::Mesh>("TargetMesh", m_targetMesh);
	AddParamBinding<ParamMode::In, ParamType::Indices>("TargetEdgeList", m_targetEdgeList);
}

bool MorphMeshLoop::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_edgeList)
	{
		Log().Error("EdgeList is empty");
		return false;
	}
	if (m_edgeList->size() < 3)
	{
		Log().Error("EdgeList must have a least three entries");
		return false;
	}
	if (!m_targetMesh)
	{
		Log().Error("TargetMesh is empty");
		return false;
	}
	if (!m_targetEdgeList)
	{
		Log().Error("TargetEdgeList is empty");
		return false;
	}
	if (m_targetEdgeList->size() < 3)
	{
		Log().Error("TargetEdgeList must have a least three entries");
		return false;
	}
	if (m_blendArea < 0)
	{
		Log().Warning("BlendArea is clamped to 0");
	}

	// first start with the absolute closest match
	float minDist = std::numeric_limits<float>::max();
	size_t minArgI = 0, minArgJ = 0;
	for (size_t i = 0; i < m_edgeList->size(); ++i)
	{
		glm::vec3& srcPt = m_mesh->vertices.at(m_edgeList->at(i));
		for (size_t j = 0; j < m_targetEdgeList->size(); ++j)
		{
			glm::vec3 const& tarPt = m_targetMesh->vertices.at(m_targetEdgeList->at(j));
			const float dist = glm::distance(srcPt, tarPt);
			if (dist < minDist)
			{
				minDist = dist;
				minArgI = i;
				minArgJ = j;
			}
		}
	}

	// debug vis
	auto connect = [&](size_t i, size_t j)
		{
			m_mesh->vertices.at(i) = m_targetMesh->vertices.at(j);
		};

	connect(m_edgeList->at(minArgI), m_targetEdgeList->at(minArgJ));

	// build the initial sequences
	std::vector<uint32_t> srcSeq;
	srcSeq.resize(m_edgeList->size() - 1);
	for (size_t i = 1; i < m_edgeList->size(); ++i)
	{
		srcSeq.at(i - 1) = m_edgeList->at((minArgI + i) % m_edgeList->size());
	}
	std::vector<uint32_t> tarSeq;
	tarSeq.resize(m_targetEdgeList->size() - 1);
	for (size_t i = 1; i < m_targetEdgeList->size(); ++i)
	{
		tarSeq.at(i - 1) = m_targetEdgeList->at((minArgJ + i) % m_targetEdgeList->size());
	}

	// TODO: check winding rule of sequences
	std::reverse(srcSeq.begin(), srcSeq.end());

	// debug -- interpolate connect (use for small sequences
	const size_t d = std::max(srcSeq.size(), tarSeq.size());
	for (size_t i = 0; i < d; ++i)
	{
		size_t si = std::clamp<size_t>(static_cast<size_t>(static_cast<double>(srcSeq.size() - 1) * static_cast<double>(i) / static_cast<double>(d - 1)), 0, srcSeq.size() - 1);
		size_t ti = std::clamp<size_t>(static_cast<size_t>(static_cast<double>(tarSeq.size() - 1) * static_cast<double>(i) / static_cast<double>(d - 1)), 0, tarSeq.size() - 1);

		connect(srcSeq.at(si), tarSeq.at(ti));
	}


	//for (size_t i = 0; i < m_edgeList->size(); ++i)
	//{

	//	glm::vec3& srcPt = m_mesh->vertices.at(m_edgeList->at(i));
	//	glm::vec3 srcPtTest = srcPt;

	//	float minDist = std::numeric_limits<float>::max();
	//	for (uint32_t tarIdx : *m_targetEdgeList)
	//	{
	//		glm::vec3 const& tarPt = m_targetMesh->vertices.at(tarIdx);
	//		const float dist = glm::distance(srcPtTest, tarPt);
	//		if (dist < minDist)
	//		{
	//			minDist = dist;
	//			srcPt = tarPt;
	//		}
	//	}

	//}

	// TODO: there will be:
	//   - degenerated triangles
	//   - holes


	// TODO: Implement

	return false;
}
