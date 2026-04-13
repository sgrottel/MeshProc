#pragma once

#include "commands/AbstractCommand.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace meshproc
{
	namespace commands
	{
		namespace compute
		{

			class LinearColorMap : public AbstractCommand
			{
			public:
				LinearColorMap(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				// TODO: add parameters to configure the color map
				const std::shared_ptr<std::vector<float>> m_scalars;
				std::shared_ptr<std::vector<glm::vec3>> m_colors;
			};

		}
	}
}
