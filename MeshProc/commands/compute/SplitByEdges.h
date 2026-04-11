#pragma once
#pragma once

#include "commands/AbstractCommand.h"
#include "data/Mesh.h"
#include "data/Scene.h"

#include <glm/glm.hpp>

#include <memory>

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
				std::shared_ptr<data::Scene> m_scene{};
				const float m_angleDeg { 30.0f };
			};

		}
	}
}
