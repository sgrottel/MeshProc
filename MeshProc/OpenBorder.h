#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class OpenBorder : public AbstractCommand
	{
	public:
		OpenBorder(const sgrottel::ISimpleLog& log);

		Parameter<std::shared_ptr<data::Mesh>, ParamMode::In> Mesh;
		Parameter<std::vector<std::vector<uint32_t>>, ParamMode::Out> EdgeLists;

		bool Invoke() override;
	};

}