#pragma once

#include "commands/AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{
	namespace commands
	{
		namespace edit
		{
			class SelectConnectedComponentVertices : public AbstractCommand
			{
			public:
				SelectConnectedComponentVertices(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::shared_ptr<data::Mesh> m_mesh;
				std::shared_ptr<std::vector<uint32_t>> m_selection;
			};

		}
	}
}
