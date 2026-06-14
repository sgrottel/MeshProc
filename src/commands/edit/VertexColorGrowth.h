#pragma once

#include "commands/AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>

namespace meshproc
{
	namespace commands
	{
		namespace edit
		{
			// inplace edit of mesh: subdivide each tri in four tris by halving each edge
			class VertexColorGrowth : public AbstractCommand
			{
			public:
				VertexColorGrowth(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::shared_ptr<data::Mesh> m_mesh;
				const glm::vec3 m_color{ 0.0f };
				const glm::vec3 m_blankColor{ 0.0f };
				float m_countColored{ 0.0f };
				std::shared_ptr<std::vector<glm::vec3>> m_colors;
			};

		}
	}
}
REGISTER_COMMAND(meshproc, commands, edit, VertexColorGrowth)
