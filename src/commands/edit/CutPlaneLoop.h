#pragma once

#include "commands/AbstractCommand.h"

#include "data/HalfSpace.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{
	namespace commands
	{
		namespace edit
		{
			class CutPlaneLoop : public AbstractCommand
			{
			public:
				CutPlaneLoop(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				std::shared_ptr<data::Mesh> m_mesh;
				const std::shared_ptr<data::HalfSpace> m_plane;
				const glm::vec3 m_point;
			};

		}
	}
}
