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
			StlWriter(sgrottel::ISimpleLog& log);

			Parameter<std::filesystem::path> Path{};
			Parameter<std::shared_ptr<data::Scene>> Scene{};

			bool Invoke() override;
		};

	}
}
