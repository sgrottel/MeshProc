#pragma once

#include "data/HashableEdge.h"

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{
	namespace utilities
	{

		// do a constraint delauney triangulation, enforcing all edges
		class Constrained2DTriangulation
		{
		public:
			// @param points -- maps from original index (of 3d point) to a 2d point
			// @param edges -- edges with indices into `points` (not original indices)
			Constrained2DTriangulation(
				const std::unordered_map<uint32_t, glm::vec2>& points,
				const std::unordered_set<data::HashableEdge>& edges,
				const sgrottel::ISimpleLog& log
				);

			// triangle winding rules NOT enforced
			std::vector<glm::uvec3> Compute() const;

			inline bool HasError() const
			{
				return m_hasError;
			}

		private:
			const std::unordered_map<uint32_t, glm::vec2> &m_points;
			const std::unordered_set<data::HashableEdge> &m_edges;
			const sgrottel::ISimpleLog& m_log;
			mutable bool m_hasError;
		};

	}
}
