#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

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
			Parameter<std::shared_ptr<data::Mesh>> Mesh{};

			bool Invoke() override;
		};

	}
}
