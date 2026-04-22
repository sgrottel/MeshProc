#pragma once

#include "commands/AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>

namespace meshproc
{
	namespace commands
	{
		namespace generator
		{

			class Cuboid : public AbstractCommand
			{
			public:
				Cuboid(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const float m_sizeX{ 1.0f };
				const float m_sizeY{ 1.0f };
				const float m_sizeZ{ 1.0f };
				const uint32_t m_segCntX{ 1 };
				const uint32_t m_segCntY{ 1 };
				const uint32_t m_segCntZ{ 1 };
				std::shared_ptr<data::Mesh> m_mesh;
			};

		}
	}
}
