#pragma once

#include "AbstractCommand.h"
#include "Mesh.h"

#include <memory>

namespace meshproc
{
	namespace generator
	{

		class Icosahedron : public AbstractCommand
		{
		public:
			Icosahedron(const sgrottel::ISimpleLog& log);

			Parameter<std::shared_ptr<Mesh>> Mesh{};

			bool Invoke() override;
		};

	}
}
