#pragma once

#include "AbstractCommand.h"
#include "data/Scene.h"

#include <filesystem>
#include <memory>

namespace meshproc
{
	namespace io
	{

		class StlWriter : public AbstractCommand
		{
		public:
			StlWriter(const sgrottel::ISimpleLog& log);

			Parameter<std::filesystem::path, ParamMode::In> Path{};
			Parameter<std::shared_ptr<data::Scene>, ParamMode::In> Scene{};

			bool Invoke() override;
		};

	}
}
