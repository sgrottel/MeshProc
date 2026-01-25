#include "Vec2Intersections.h"

namespace
{

	// Returns +1 if counter-clockwise, -1 if clockwise, 0 if colinear
	int orientation(const glm::vec2& p, const glm::vec2& q, const glm::vec2& r) {
		double val = (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
		if (val > 0) return 1;     // left turn
		if (val < 0) return -1;    // right turn
		return 0;                  // colinear
	}

	// Checks if r lies on segment pq
	bool on_segment(const glm::vec2& p, const glm::vec2& q, const glm::vec2& r) {
		return (std::min)(p.x, q.x) <= r.x && r.x <= (std::max)(p.x, q.x)
			&& (std::min)(p.y, q.y) <= r.y && r.y <= (std::max)(p.y, q.y);
	}

	float cross(const glm::vec2& u, const glm::vec2& v) {
		return u.x * v.y - u.y * v.x;
	}

}

// Main intersection test
bool meshproc::utilities::vec2segments::DoesIntersect(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& d) {
	int o1 = orientation(a, b, c);
	int o2 = orientation(a, b, d);
	int o3 = orientation(c, d, a);
	int o4 = orientation(c, d, b);

	// General case
	if (o1 != o2 && o3 != o4) return true;

	// Colinear special cases
	if (o1 == 0 && on_segment(a, b, c)) return true;
	if (o2 == 0 && on_segment(a, b, d)) return true;
	if (o3 == 0 && on_segment(c, d, a)) return true;
	if (o4 == 0 && on_segment(c, d, b)) return true;

	return false;
}

glm::vec2 meshproc::utilities::vec2segments::CalcIntersection(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& d)
{
	glm::vec2 r = b - a;
	glm::vec2 s = d - c;
	float denom = cross(r, s);
	assert(denom != 0.0f); // nonzero because they intersect
	float t = cross(c - a, s) / denom;
	return a + t * r;
}