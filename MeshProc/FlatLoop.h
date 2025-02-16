#pragma once

#include "Mesh.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace sgrottel
{
	class ISimpleLog;
}

class FlatLoop
{
public:
	FlatLoop(sgrottel::ISimpleLog& log);

	template<typename C>
	std::vector<glm::vec2> Project(C const& vertices, std::vector<uint32_t> const& loop, glm::vec3 center, glm::vec3 x2d, glm::vec3 y2d) const;

	std::vector<glm::uvec2> IsSelfintersecting(std::vector<glm::vec2> l) const;

private:
	sgrottel::ISimpleLog& m_log;
};

template<typename C>
std::vector<glm::vec2> FlatLoop::Project(C const& vertices, std::vector<uint32_t> const& loop, glm::vec3 center, glm::vec3 x2d, glm::vec3 y2d) const
{
	std::vector<glm::vec2> v2d;
	v2d.resize(loop.size());
	std::transform(
		loop.begin(),
		loop.end(),
		v2d.begin(),
		[&](uint32_t i)
		{
			const glm::vec3 v = vertices[i] - center;
			return glm::vec2{ glm::dot(v, x2d), glm::dot(v, y2d) };
		});
	return v2d;
}
