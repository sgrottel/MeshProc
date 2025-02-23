#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

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

			Parameter<std::filesystem::path, ParamType::In> Path{};
			Parameter<std::shared_ptr<data::Mesh>, ParamType::Out> Mesh{};

			bool Invoke() override;
		};

	}
}
