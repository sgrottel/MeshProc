#pragma once

#include "AbstractCommand.h"

#include "data/Mesh.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace meshproc
{

	class VertexSurfaceDistanceToCut : public AbstractCommand
	{
	public:
		VertexSurfaceDistanceToCut(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<data::Mesh> m_mesh;
		const glm::vec3 m_planeOrig{ 0.0f, 0.0f, 0.0f };
		const glm::vec3 m_planeNorm{ 0.0f, 0.0f, 1.0f };
		const glm::vec3 m_planeX{ 1.0f, 0.0f, 0.0f };
		const float m_planeRectWidth{ 0.0f };
		const float m_planeRectHeight{ 0.0f };

		std::shared_ptr<std::vector<float>> m_dists;
	};

}
