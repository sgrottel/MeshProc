#pragma once

#include "AbstractCommand.h"
#include "Mesh.h"

#include <memory>

namespace meshproc
{

	class CubeGenerator : public AbstractCommand
	{
	public:
		CubeGenerator(const sgrottel::ISimpleLog& log);

		Parameter<float> SizeX{};
		Parameter<float> SizeY{};
		Parameter<float> SizeZ{};
		Parameter<std::shared_ptr<Mesh>> Mesh{};

		bool Invoke() override;
	};

}
