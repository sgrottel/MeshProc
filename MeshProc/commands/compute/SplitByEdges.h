#pragma once
#pragma once

#include "commands/AbstractCommand.h"
#include "data/Mesh.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace meshproc
{
	namespace commands
	{
		namespace compute
		{

			class SplitByEdges : public AbstractCommand
			{
			public:
				SplitByEdges(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::shared_ptr<data::Mesh> m_mesh{};
				std::shared_ptr<std::vector<std::shared_ptr<data::Mesh>>> m_segments{};
				const float m_angleDeg { 30.0f };
			};

		}
	}
}
