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

			Parameter<float, ParamType::In> SizeX;
			Parameter<float, ParamType::In> SizeY;
			Parameter<float, ParamType::In> SizeZ;
			Parameter<std::shared_ptr<data::Mesh>, ParamType::Out> Mesh;

			bool Invoke() override;
		};

	}
}
