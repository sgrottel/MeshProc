#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace meshproc
{
	namespace fitting
	{

		class MorphRotateOutline : public AbstractCommand
		{
		public:
			MorphRotateOutline(const sgrottel::ISimpleLog& log);
			bool Invoke() override;

		private:
			std::vector<uint32_t> GetMeshBorderVertices() const;

			std::shared_ptr<data::Mesh> m_mesh;
			const std::shared_ptr<data::Mesh> m_targetMesh;
			const glm::vec3 m_center{ 0.0f };
			const float m_fixRadius{ 0.0f };
			const glm::vec3 m_primaryAxis{ 0.0f, 1.0f, 0.0f };
		};
	}
}
