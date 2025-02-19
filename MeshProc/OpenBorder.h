#pragma once

#include "AbstractCommand.h"
#include "Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class OpenBorder : public AbstractCommand
	{
	public:
		OpenBorder(sgrottel::ISimpleLog& log);

		Parameter<std::shared_ptr<Mesh>> Mesh;
		Parameter<std::vector<std::vector<uint32_t>>> EdgeLists;

		bool Invoke() override;
	};

}