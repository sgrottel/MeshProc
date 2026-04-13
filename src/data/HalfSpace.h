#pragma once

#include "HashableEdge.h"

#include <glm/glm.hpp>

#include <tuple>

namespace sgrottel
{
	class ISimpleLog;
}

namespace meshproc
{
	namespace data
	{
		class HalfSpace
		{
		public:

			bool Set(const glm::vec3& normal, float dist)
			{
				if (normal.x == 0 && normal.y == 0 && normal.z == 0)
				{
					return false;
				}

				m_normal = glm::normalize(normal);
				m_dist = dist;

				return true;
			}

			bool Set(const glm::vec3& normal, const glm::vec3& point)
			{
				if (normal.x == 0 && normal.y == 0 && normal.z == 0)
				{
					return false;
				}

				m_normal = glm::normalize(normal);
				m_dist = glm::dot(m_normal, point);

				return true;
			}

			inline glm::vec3 const& Normal() const noexcept
			{
				return m_normal;
			}
			inline glm::vec3 const Plane() const noexcept
			{
				return m_normal * m_dist;
			}
			inline float Dist() const noexcept
			{
				return m_dist;
			}

			// signed dist of vertex v to plane
			inline float Dist(glm::vec3 const& v) const
			{
				return glm::dot(v, m_normal) - m_dist;
			}

			template<typename CT>
			inline bool IsCut(HashableEdge const& edge, CT const& distContainer) const
			{
				return std::signbit(distContainer.at(edge.start)) != std::signbit(distContainer.at(edge.end));
			}

			template<typename DCT, typename VCT>
			inline glm::vec3 CutInterpolate(HashableEdge const& edge, DCT const& distContainer, VCT const& vertexContainer) const
			{
				const float adi = abs(distContainer.at(edge.start));
				const float adj = abs(distContainer.at(edge.end));
				const float b = adi / (adi + adj);
				const float a = adj / (adi + adj);
				return vertexContainer.at(edge.start) * a + vertexContainer.at(edge.end) * b;
			}

			std::tuple<glm::vec3, glm::vec3> Make2DCoordSys() const;

		private:
			glm::vec3 m_normal{ 0.0f, 0.0f, 1.0f };
			float m_dist{ 0.0f };
		};
	}
}
