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
			inline const glm::vec3& GetPlaneNormalParam() const noexcept
			{
				return m_planeNormalParam;
			}
			inline const float& GetPlaneDistParam() const noexcept
			{
				return m_planeDistParam;
			}

			bool ValidateParams(sgrottel::ISimpleLog const& log);

			inline glm::vec3 const& Normal() const noexcept
			{
				return m_normal;
			}
			inline glm::vec3 const& Plane() const noexcept
			{
				return m_plane;
			}
			inline float Dist() const noexcept
			{
				return m_planeDistParam;
			}

			// signed dist of vertex v to plane
			inline float Dist(glm::vec3 const& v) const
			{
				return glm::dot(v - m_plane, m_normal);
			}

			template<typename CT>
			inline bool IsCut(HashableEdge const& edge, CT const& distContainer)
			{
				return std::signbit(distContainer.at(edge.start)) != std::signbit(distContainer.at(edge.end));
			}

			template<typename DCT, typename VCT>
			inline glm::vec3 CutInterpolate(HashableEdge const& edge, DCT const& distContainer, VCT const& vertexContainer)
			{
				const float adi = abs(distContainer.at(edge.start));
				const float adj = abs(distContainer.at(edge.end));
				const float b = adi / (adi + adj);
				const float a = adj / (adi + adj);
				return vertexContainer.at(edge.start) * a + vertexContainer.at(edge.end) * b;
			}

			std::tuple<glm::vec3, glm::vec3> Make2DCoordSys() const;

		private:
			const glm::vec3 m_planeNormalParam{ 0.0f, 0.0f, 1.0f };
			const float m_planeDistParam{ 0.0f };
			glm::vec3 m_normal{};
			glm::vec3 m_plane{};
		};
	}
}
