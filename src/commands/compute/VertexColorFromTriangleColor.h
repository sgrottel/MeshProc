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
			class VertexColorFromTriangleColor : public AbstractCommand
			{
			public:
				VertexColorFromTriangleColor(const sgrottel::ISimpleLog& log);
				bool Invoke() override;
			private:
				const std::shared_ptr<data::Mesh> m_mesh;
				const std::shared_ptr<std::vector<float>> m_tricol;
				std::shared_ptr<std::vector<float>> m_vertcol;
			};
		}
	}
}
