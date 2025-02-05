#include "Mesh.h"

#include <cmath>

bool Mesh::IsValid() const
{
	for (glm::vec3 const& v : vertices) {
		for (int i = 0; i < 3; ++i) {
			if (std::isnan(v[i])) return false;
		}
	}
	for (glm::uvec3 const& t : triangles) {
		for (int i = 0; i < 3; ++i) {
			if (t[i] >= vertices.size()) return false;
		}
		if (t.x == t.y || t.y == t.z || t.x == t.z) {
			return false;
		}
	}
}
