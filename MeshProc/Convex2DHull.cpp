#include "Convex2DHull.h"

#include <SimpleLog/SimpleLog.hpp>

#include <glm/glm.hpp>

#include <array>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace meshproc;

Convex2DHull::Convex2DHull(const sgrottel::ISimpleLog& log)
	: AbstractCommand{ log }
	, m_loops{ nullptr }
	, m_hull{ nullptr }
{
	AddParamBinding<ParamMode::In, ParamType::Shape2D>("Loops", m_loops);
	AddParamBinding<ParamMode::Out, ParamType::Shape2D>("Hull", m_hull);
}

namespace
{
	inline float cross(const glm::vec2& O, const glm::vec2& A, const glm::vec2& B) {
		return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
	}
}

bool Convex2DHull::Invoke()
{
	if (!m_loops)
	{
		Log().Error("Mesh not set");
		return false;
	}

	std::vector<glm::vec2> points;
	points.reserve(0);
	for (auto const& l : m_loops->loops)
	{
		points.reserve(points.capacity() + l.second.size());
	}
	for (auto const& l : m_loops->loops)
	{
		for (auto const& p : l.second)
		{
			points.push_back(p);
		}
	}

	m_hull = std::make_shared<data::Shape2D>();

	size_t n = points.size(), k = 0;
	if (n <= 3)
	{
		m_hull->loops[0] = points;
		return true;
	}

	auto& hull = m_hull->loops[0];
	hull.resize(2 * n);

	// Sort points lexicographically
	std::sort(points.begin(), points.end(), [](const glm::vec2& a, const glm::vec2& b) {
		if (a.x == b.x)
		{
			return a.y < b.y;
		}
		return a.x < b.x;
		});

	// Build lower hull
	for (size_t i = 0; i < n; ++i) {
		while (k >= 2 && cross(hull[k - 2], hull[k - 1], points[i]) <= 0) k--;
		hull[k++] = points[i];
	}

	// Build upper hull
	for (size_t i = n - 1, t = k + 1; i > 0; --i) {
		while (k >= t && cross(hull[k - 2], hull[k - 1], points[i - 1]) <= 0) k--;
		hull[k++] = points[i - 1];
	}

	hull.resize(k - 1); // Remove duplicate endpoint

	return true;
}
