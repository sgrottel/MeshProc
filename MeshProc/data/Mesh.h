#pragma once

#include "data/Triangle.h"

#include <glm/glm.hpp>

#include <unordered_set>
#include <vector>

namespace meshproc
{
	namespace data
	{

		class Mesh
		{
		public:

			std::vector<glm::vec3> vertices;
			std::vector<Triangle> triangles;

			inline void AddQuad(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4)
			{
				triangles.push_back(Triangle{ i1, i2, i3 });
				triangles.push_back(Triangle{ i3, i2, i4 });
			}

			// Checks for structural validity.
			// Does not include checking for:
			// - T-vertices
			// - non-manifolds (edges used by != 2 triangles)
			// - congruent vertices and thus degenerated triangles
			bool IsValid() const;

			std::unordered_set<data::HashableEdge> CollectOpenEdges() const;
		};

	}
}