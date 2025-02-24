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

			Parameter<float, ParamMode::In> SizeX;
			Parameter<float, ParamMode::In> SizeY;
			Parameter<float, ParamMode::In> SizeZ;
			Parameter<std::shared_ptr<data::Mesh>, ParamMode::Out> Mesh;

			bool Invoke() override;
		};

	}
}
