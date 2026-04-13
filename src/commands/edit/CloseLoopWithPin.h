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
			class CloseLoopWithPin : public AbstractCommand
			{
			public:
				CloseLoopWithPin(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				std::shared_ptr<data::Mesh> m_mesh;
				const std::shared_ptr<std::vector<uint32_t>> m_loop;
				uint32_t m_newVertexIndex;
			};

		}
	}
}
