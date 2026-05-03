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
			class BlowupFill : public AbstractCommand
			{
			public:
				BlowupFill(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				std::shared_ptr<data::Mesh> m_mesh;
				const glm::vec3 m_point { 0.0f, 0.0f, 0.0f };
			};

		}
	}
}
