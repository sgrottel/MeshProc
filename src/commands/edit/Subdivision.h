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
			class Subdivision : public AbstractCommand
			{
			public:
				Subdivision(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				std::shared_ptr<data::Mesh> m_mesh;
				std::shared_ptr<std::vector<uint32_t>> m_newVertices;
			};

		}
	}
}
