#pragma once

#include "Triangle.h"

#include <glm/glm.hpp>
#include <vector>

class Mesh
{
public:

	std::vector<glm::vec3> vertices;
	std::vector<Triangle> triangles;

	// Checks for structural validity.
	// Does not include checking for:
	// - T-vertices
	// - non-manifolds (edges used by != 2 triangles)
	// - congruent vertices and thus degenerated triangles
	bool IsValid() const;
};
