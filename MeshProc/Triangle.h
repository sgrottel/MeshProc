#pragma once

#include <cstdint>
#include <stdexcept>

#include <glm/glm.hpp>

class Triangle
{
public:
	Triangle(uint32_t i0 = 0, uint32_t i1 = 0, uint32_t i2 = 0);

	inline uint32_t& operator[](size_t i)
	{
		if (i < 0 || i >= 3) throw std::out_of_range("index must be [0..3[");
		return m_idx[i];
	}
	inline const uint32_t& operator[](size_t i) const
	{
		if (i < 0 || i >= 3) throw std::out_of_range("index must be [0..3[");
		return m_idx[i];
	}

	template<typename C>
	glm::vec3 CalcNormal(C const& vertices) const;

private:
	uint32_t m_idx[3];

};

template<typename C>
glm::vec3 Triangle::CalcNormal(C const& vertices) const
{
	const glm::vec3 v0 = vertices[m_idx[0]];
	const glm::vec3 v1 = vertices[m_idx[1]];
	const glm::vec3 v2 = vertices[m_idx[2]];
	return glm::normalize(glm::cross(v0 - v1, v1 - v2));
}
