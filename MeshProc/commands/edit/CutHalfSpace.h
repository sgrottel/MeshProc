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
			class CutHalfSpace : public AbstractCommand
			{
			public:
				CutHalfSpace(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				std::shared_ptr<data::Mesh> m_mesh;
				const std::shared_ptr<data::HalfSpace> m_halfSpace;
				std::shared_ptr<std::vector<std::shared_ptr<std::vector<uint32_t>>>> m_openLoops;
			};

		}
	}
}
