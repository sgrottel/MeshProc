#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>

namespace meshproc
{
	namespace generator
	{

		class Icosahedron : public AbstractCommand
		{
		public:
			Icosahedron(const sgrottel::ISimpleLog& log);

			bool Invoke() override;

		protected:
			std::shared_ptr<data::Mesh> m_mesh{};
		};

	}
}
