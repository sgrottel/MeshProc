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

			class OutsideSurfaceClassification : public AbstractCommand
			{
			public:
				OutsideSurfaceClassification(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::shared_ptr<data::Mesh> m_mesh;
				std::shared_ptr<std::vector<float>> m_facetype;
			};

		}
	}
}
