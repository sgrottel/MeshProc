#pragma once

#include <cstdint>
#include <stdexcept>

#include <glm/glm.hpp>

namespace meshproc
{
	namespace data
	{

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

			inline void Flip()
			{
				std::swap(m_idx[1], m_idx[2]);
			}

			template<typename C>
			bool OrientationMatches(C const& vertices, Triangle const& tri, glm::uvec2 commonEdge) const;

			inline bool HasIndex(uint32_t idx) const
			{
				for (int i = 0; i < 3; ++i)
				{
					if (m_idx[i] == idx)
					{
						return true;
					}
				}
				return false;
			}

			inline uint32_t ThirdIndex(uint32_t i0, uint32_t i1) const
			{
				for (int i = 0; i < 3; ++i)
				{
					if (m_idx[i] != i0 && m_idx[i] != i1)
					{
						return m_idx[i];
					}
				}
				return m_idx[0];
			}

			inline glm::uvec2 CommonEdge(Triangle const& t) const
			{
				if (t.HasIndex(m_idx[0]))
				{
					if (t.HasIndex(m_idx[1]))
					{
						if (t.HasIndex(m_idx[2]))
						{
							return { m_idx[0], m_idx[0] };
						}
						else
						{
							return { m_idx[0], m_idx[1] };
						}
					}
					else
					{
						if (t.HasIndex(m_idx[2]))
						{
							return { m_idx[0], m_idx[2] };
						}
						else
						{
							return { m_idx[0], m_idx[0] };
						}
					}
				}
				else
				{
					if (t.HasIndex(m_idx[1]))
					{
						if (t.HasIndex(m_idx[2]))
						{
							return { m_idx[1], m_idx[2] };
						}
						else
						{
							return { m_idx[1], m_idx[1] };
						}
					}
					else
					{
						if (t.HasIndex(m_idx[2]))
						{
							return { m_idx[2], m_idx[2] };
						}
						else
						{
							return { 0, 0 };
						}
					}
				}
			}

			inline glm::uvec2 HashableEdge(uint32_t idx) const
			{
				glm::uvec2 e{ m_idx[idx], m_idx[(idx + 1) % 3] };
				if (e.x > e.y)
				{
					std::swap(e.x, e.y);
				}
				return e;
			}

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

		template<typename C>
		bool Triangle::OrientationMatches(C const& vertices, Triangle const& tri, glm::uvec2 commonEdge) const
		{
			assert(HasIndex(commonEdge.x));
			assert(HasIndex(commonEdge.y));
			assert(tri.HasIndex(commonEdge.x));
			assert(tri.HasIndex(commonEdge.y));

			const glm::vec3 v0 = vertices[commonEdge.x];

			const glm::vec3 v1 = vertices[ThirdIndex(commonEdge.x, commonEdge.y)] - v0;
			const glm::vec3 v2 = vertices[tri.ThirdIndex(commonEdge.x, commonEdge.y)] - v0;

			const glm::vec3 n1 = CalcNormal(vertices);
			const glm::vec3 n2 = tri.CalcNormal(vertices);

			const float nAligned = glm::dot(n1, n2);
			if (nAligned > 0.99)
			{
				// triangles are flat and normals point into the same direction
				return true;
			}

			const float d1 = glm::dot(n1, v2);
			const float d2 = glm::dot(n2, v1);

			const bool neg1 = d1 < 0.0f;
			const bool neg2 = d2 < 0.0f;

			return neg1 == neg2;
		}
	}
}
