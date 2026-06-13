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
			class VertexColorCleanup : public AbstractCommand
			{
			public:
				VertexColorCleanup(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::shared_ptr<data::Mesh> m_mesh;
				std::shared_ptr<std::vector<glm::vec3>> m_colors;
			};

		}
	}
}
REGISTER_COMMAND(meshproc, commands, edit, VertexColorCleanup)
