#pragma once

#include "AbstractCommand.h"
#include "Mesh.h"

#include <memory>
#include <filesystem>

namespace meshproc
{
	namespace io
	{

		class StlReader : public AbstractCommand
		{
		public:
			StlReader(sgrottel::ISimpleLog& log);

			Parameter<std::filesystem::path> Path{};
			Parameter<std::shared_ptr<Mesh>> Mesh{};

			bool Invoke() override;
		};

	}
}
