#include "MixLoops.h"

#include <SimpleLog/SimpleLog.hpp>

#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using namespace meshproc;

MixLoops::MixLoops(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("MeshA", m_meshA);
	AddParamBinding<ParamMode::In, ParamType::Indices>("EdgeListA", m_edgeListA);
	AddParamBinding<ParamMode::In, ParamType::Mesh>("MeshB", m_meshB);
	AddParamBinding<ParamMode::In, ParamType::Indices>("EdgeListB", m_edgeListB);
	AddParamBinding<ParamMode::In, ParamType::Float>("MixFactor", m_mixFactor);
	AddParamBinding<ParamMode::Out, ParamType::Mesh>("OutMesh", m_outMesh);
	AddParamBinding<ParamMode::Out, ParamType::Indices>("OutEdgeList", m_outEdgeList);
}

bool MixLoops::Invoke()
{
	if (!m_meshA)
	{
		Log().Error("MeshA is empty");
		return false;
	}
	if (!m_edgeListA)
	{
		Log().Error("EdgeListA is empty");
		return false;
	}
	if (m_edgeListA->size() < 3)
	{
		Log().Error("EdgeListA must have a least three entries");
		return false;
	}
	if (!m_meshB)
	{
		Log().Error("MeshB is empty");
		return false;
	}
	if (!m_edgeListB)
	{
		Log().Error("EdgeListB is empty");
		return false;
	}
	if (m_edgeListB->size() < 3)
	{
		Log().Error("EdgeListB must have a least three entries");
		return false;
	}
	if (m_mixFactor < 0)
	{
		Log().Warning("MixFactor is clamped to 0");
	}
	if (m_mixFactor > 1)
	{
		Log().Warning("MixFactor is clamped to 1");
	}

	m_outMesh = std::make_shared<data::Mesh>();

	const size_t outSize = std::min(m_edgeListA->size(), m_edgeListB->size());

	m_outMesh->vertices.resize(outSize);

	const std::shared_ptr<std::vector<uint32_t>> smallLoop = (m_edgeListA->size() == outSize) ? m_edgeListA : m_edgeListB;
	const std::shared_ptr<data::Mesh> smallLoopMesh = (smallLoop == m_edgeListA) ? m_meshA : m_meshB;

	const std::shared_ptr<std::vector<uint32_t>> largeLoop = (smallLoop == m_edgeListA) ? m_edgeListB : m_edgeListA;
	const std::shared_ptr<data::Mesh> largeLoopMesh = (smallLoop == m_edgeListA) ? m_meshB : m_meshA;

	std::transform(smallLoop->begin(), smallLoop->end(), m_outMesh->vertices.begin(), [smallLoopMesh](uint32_t i) { return smallLoopMesh->vertices.at(i); });

	std::vector<glm::vec4> pts;
	pts.resize(outSize);

	std::fill(pts.begin(), pts.end(), glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f });

	for (uint32_t i : *largeLoop)
	{
		glm::vec3 p = largeLoopMesh->vertices.at(i);

		float minDist = std::numeric_limits<float>::max();
		size_t minIdx = -1;
		for (size_t j = 0; j < outSize; ++j)
		{
			float d = glm::distance(p, m_outMesh->vertices.at(j));
			if (d < minDist)
			{
				minDist = d;
				minIdx = j;
			}
		}
		if (minIdx < 0) throw std::runtime_error("no min dist point found");

		pts.at(minIdx) += glm::vec4{ p, 1.0 };
	}

	bool hasValues = false;
	bool hasZeros = false;
	for (size_t j = 0; j < outSize; ++j)
	{
		auto& p = pts.at(j);

		if (p.w < 0.5f)
		{
			hasZeros = true;
			continue;
		}

		hasValues = true;

		if (p.w > 1.5f)
		{
			p /= p.w;
		}
	}

	if (!hasValues) throw std::runtime_error("no position interpolation found");

	if (hasZeros)
	{
		for (size_t j = 0; j < outSize; ++j)
		{
			auto& p = pts.at(j);
			if (p.w > 0.5f) continue;

			int32_t pd = 0, nd = 0;
			size_t npi = 0, ppi = 0;
			for (; nd < static_cast<int32_t>(outSize); ++nd)
			{
				npi = (j + nd) % outSize;
				if (pts.at(npi).w > 0.5f) break;
			}
			for (; pd < static_cast<int32_t>(outSize); ++pd)
			{
				ppi = (j + outSize - pd) % outSize;
				if (pts.at(ppi).w > 0.5f) break;
			}

			const float a = static_cast<float>(npi) / static_cast<float>(npi + ppi);
			assert(0.0f <= a && a <= 1.0f);
			const float b = 1.0f - a;

			p = pts.at(ppi) * a + pts.at(npi) * b;
		}
	}

	const float b = std::clamp(m_mixFactor, 0.0f, 1.0f);
	const float a = 1.0f - b;
	for (size_t i = 0; i < outSize; ++i)
	{
		glm::vec3& v = m_outMesh->vertices.at(i);
		glm::vec4& p = pts.at(i);
		v = v * a + glm::vec3{ p.x * b, p.y * b, p.z * b };
	}

	// check for degenerated edges
	for (size_t i = 1; i < m_outMesh->vertices.size(); ++i)
	{
		glm::vec3& v1 = m_outMesh->vertices.at(i - 1);
		glm::vec3& v2 = m_outMesh->vertices.at(i);

		if (glm::distance(v1, v2) < 0.00001f)
		{
			m_outMesh->vertices.erase(m_outMesh->vertices.begin() + i);
			i--;
		}
	}

	m_outEdgeList = std::make_shared<std::vector<uint32_t>>();
	m_outEdgeList->resize(m_outMesh->vertices.size());
	for (uint32_t i = 0; i < m_outMesh->vertices.size(); ++i)
	{
		m_outEdgeList->at(i) = i;
	}

#ifdef _DEBUG
	// DEBUG: Add mesh for visualization
	const size_t s = m_outMesh->vertices.size();
	m_outMesh->vertices.resize(s * 2);
	for (size_t i = 0; i < s; ++i)
	{
		m_outMesh->vertices.at(i + s) = m_outMesh->vertices.at(i) + glm::vec3{ 0.0f, 0.0f, 2.0f };
	}

	m_outMesh->triangles.reserve(s * 2);
	for (size_t i = 0; i < s; ++i)
	{
		size_t i2 = (i + 1) % s;
		m_outMesh->AddQuad(
			static_cast<uint32_t>(i),
			static_cast<uint32_t>(i2),
			static_cast<uint32_t>(i + s),
			static_cast<uint32_t>(i2 + s));
	}
#endif

	return true;
}
