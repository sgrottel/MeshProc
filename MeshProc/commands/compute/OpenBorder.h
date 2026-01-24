#pragma once

#include "commands/AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{
	namespace commands
	{
		namespace compute
		{

			class OpenBorder : public AbstractCommand
			{
			public:
				OpenBorder(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::shared_ptr<data::Mesh> m_mesh;
				std::shared_ptr<std::vector<std::shared_ptr<std::vector<uint32_t>>>> m_edgeLists;
			};

		}
	}
}
