#pragma once

#include "commands/AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>

namespace meshproc
{
	namespace commands
	{
		namespace generator
		{

			class Octahedron : public AbstractCommand
			{
			public:
				Octahedron(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			protected:
				std::shared_ptr<data::Mesh> m_mesh{};
			};

		}
	}
}
