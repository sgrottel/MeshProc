#include "Mesh.h"

#include <cmath>

bool Mesh::IsValid() const
{
	for (glm::vec3 const& v : vertices) {
		for (int i = 0; i < 3; ++i) {
			if (std::isnan(v[i])) return false;
		}
	}
	for (Triangle const& t : triangles) {
		for (int i = 0; i < 3; ++i) {
			if (t[i] >= vertices.size()) return false;
		}
		if (t[0] == t[1] || t[1] == t[2] || t[0] == t[2]) {
			return false;
		}
	}

	return true;
}
