#pragma once

#include "commands/AbstractCommand.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace meshproc
{
	namespace commands
	{
		namespace io
		{

			class VertexColorReader : public AbstractCommand
			{
			public:
				VertexColorReader(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::wstring m_path{};
				std::shared_ptr<std::vector<glm::vec3>> m_colors{};
			};

		}
	}
}
REGISTER_COMMAND(meshproc, commands, io, VertexColorReader)
