#pragma once

#include "AbstractCommand.h"
#include "data/Scene.h"

#include <filesystem>
#include <memory>

namespace meshproc
{
	namespace io
	{

		class ObjReader : public AbstractCommand
		{
		public:
			ObjReader(sgrottel::ISimpleLog& log);

			Parameter<std::filesystem::path> Path{};
			Parameter<std::shared_ptr<data::Mesh>> Mesh{};

			bool Invoke() override;
		};

	}
}
