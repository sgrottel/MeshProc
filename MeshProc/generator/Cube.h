#pragma once

#include "AbstractCommand.h"
#include "Mesh.h"

#include <memory>

namespace meshproc
{
	namespace generator
	{

		class Cube: public AbstractCommand
		{
		public:
			Cube(const sgrottel::ISimpleLog& log);

			Parameter<float> SizeX{};
			Parameter<float> SizeY{};
			Parameter<float> SizeZ{};
			Parameter<std::shared_ptr<Mesh>> Mesh{};

			bool Invoke() override;
		};

	}
}
