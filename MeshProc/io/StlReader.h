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
			StlReader(const sgrottel::ISimpleLog& log);

			Parameter<std::filesystem::path, ParamMode::In> Path{};
			Parameter<std::shared_ptr<data::Mesh>, ParamMode::Out> Mesh{};

			bool Invoke() override;
		};

	}
}
