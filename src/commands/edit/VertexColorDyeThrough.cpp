#include "VertexColorDyeThrough.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4267)
#endif
#include <flann/flann.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <SimpleLog/SimpleLog.hpp>

#include <algorithm>
#include <execution>
#include <ranges>
#include <unordered_map>

using namespace meshproc;
using namespace meshproc::commands;
using namespace meshproc::commands::edit;

namespace
{
	inline const float MHD(const glm::vec3& a, const glm::vec3& b) noexcept
	{
		return std::max(std::max(
				std::abs(a.x - b.x),
				std::abs(a.y - b.y)),
				std::abs(a.z - b.z));
	}
}

VertexColorDyeThrough::VertexColorDyeThrough(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
{
	AddParamBinding<ParamMode::In, ParamType::Mesh>("Mesh", m_mesh);
	AddParamBinding<ParamMode::In, ParamType::Vec3List>("Normals", m_normals);
	AddParamBinding<ParamMode::In, ParamType::Vec3>("BlankColor", m_blankColor);
	AddParamBinding<ParamMode::In, ParamType::Float>("MaxDist", m_maxDist);
	AddParamBinding<ParamMode::InOut, ParamType::Vec3List>("Colors", m_colors);
}

bool VertexColorDyeThrough::Invoke()
{
	if (!m_mesh)
	{
		Log().Error("Mesh is empty");
		return false;
	}
	if (!m_normals)
	{
		Log().Error("Normals is empty");
		return false;
	}
	if (m_normals->size() != m_mesh->vertices.size())
	{
		Log().Error("Normals and Mesh are not the same size");
		return false;
	}
	if (!m_colors)
	{
		Log().Error("Colors is empty");
		return false;
	}
	if (m_colors->size() != m_mesh->vertices.size())
	{
		Log().Error("Colors and Mesh are not the same size");
		return false;
	}
	if (m_maxDist < 0.0001f)
	{
		Log().Error("MaxDist must be positiv larger zero");
		return false;
	}

	Log().Detail("Dye Through blank vertices...");

	const size_t len = m_colors->size();

	std::vector<bool> isBlank(len, false);
	std::transform(m_colors->begin(), m_colors->end(), isBlank.begin(), [&](glm::vec3 const& c) { return MHD(c, m_blankColor) < 0.005f; });
	const uint32_t blanks = static_cast<uint32_t>(std::count_if(isBlank.begin(), isBlank.end(), [](auto b) { return b; }));

	std::vector<uint32_t> origIdx;
	origIdx.reserve(len - blanks);
	std::vector<float> nonBlankPt;
	nonBlankPt.reserve((len - blanks) * 3);
	for (size_t i = 0; i < len; ++i)
	{
		if (isBlank[i]) continue;
		origIdx.push_back(static_cast<uint32_t>(i));
		const glm::vec3 p = m_mesh->vertices.at(i);
		nonBlankPt.push_back(p.x);
		nonBlankPt.push_back(p.y);
		nonBlankPt.push_back(p.z);
	}
	flann::Matrix<float> nonBlankPts(nonBlankPt.data(), origIdx.size(), 3);
	flann::Index<flann::L2<float>> nonBlankPtsIndex(nonBlankPts, flann::KDTreeIndexParams(4));
	nonBlankPtsIndex.buildIndex();

	const float searchRadSqrd = m_maxDist * m_maxDist;

	std::atomic<uint32_t> colored{ 0 };

	auto index = std::views::iota(size_t{ 0 }, len);
	std::for_each(std::execution::par,
		index.begin(),
		index.end(),
		[&](size_t i)
		{
			if (!isBlank[i]) return;
			const glm::vec3 p = m_mesh->vertices.at(i);
			const glm::vec3 n = m_normals->at(i);
			// assert(std::abs(glm::length(n) - 1.0f) < 0.00001f);

			// Query point
			float query_raw[3] = { p.x, p.y, p.z };
			flann::Matrix<float> query(query_raw, 1, 3);

			// Output containers
			std::vector<std::vector<int>> indices;
			std::vector<std::vector<float>> dists;

			int cnt = nonBlankPtsIndex.radiusSearch(query, indices, dists, searchRadSqrd, flann::SearchParams(64));

			if (cnt > 0)
			{
				float minD = std::numeric_limits<float>::max();
				uint32_t minJ = static_cast<uint32_t>(len);

				for (int nidx = 0; nidx < cnt; ++nidx)
				{
					const int j = origIdx.at(indices[0].at(nidx));
					const glm::vec3 dj = m_mesh->vertices.at(j) - p;
					const glm::vec3 nj = m_normals->at(j);

					// alignment of normals: only if < -0.5, the normal point in different directions, and only then they are relevant
					const float na = glm::dot(n, nj);
					if (na > -0.5f) continue;

					// placement, only use points in the negative half space
					const float z = glm::dot(n, dj);
					if (z > -0.01f) continue;

					const float dl2 = dists[0].at(nidx);
					if (minD > dl2)
					{
						minD = dl2;
						minJ = j;
					}
				}

				if (minJ < len)
				{
					colored++;
					m_colors->at(i) = m_colors->at(minJ);
				}
			}
		});

	Log().Detail("Found %u blanks and colored %u (%f)", blanks, colored, static_cast<float>(colored) / static_cast<float>(std::max<uint32_t>(1, blanks)));

	return true;
}
