#pragma once

#include "AbstractCommand.h"
#include "Scene.h"

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
			Parameter<std::shared_ptr<Scene>> Scene{};

			bool Invoke() override;
		};

	}
}
