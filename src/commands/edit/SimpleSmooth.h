#pragma once

#include "commands/AbstractCommand.h"

#include "data/Mesh.h"

#include <glm/glm.hpp>

namespace meshproc
{
	namespace commands
	{
		namespace edit
		{
			class SimpleSmooth : public AbstractCommand
			{
			public:
				SimpleSmooth(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				std::shared_ptr<data::Mesh> m_mesh;
				const float m_strength{ 0.75f };
			};

		}
	}
}
