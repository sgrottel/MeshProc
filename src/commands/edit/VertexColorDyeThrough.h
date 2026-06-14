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
			class VertexColorDyeThrough : public AbstractCommand
			{
			public:
				VertexColorDyeThrough(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::shared_ptr<data::Mesh> m_mesh;
				const std::shared_ptr<std::vector<glm::vec3>> m_normals;
				const glm::vec3 m_blankColor{ 0.0f };
				const float m_maxDist{ 1.0f };
				std::shared_ptr<std::vector<glm::vec3>> m_colors;
			};

		}
	}
}
REGISTER_COMMAND(meshproc, commands, edit, VertexColorDyeThrough)
