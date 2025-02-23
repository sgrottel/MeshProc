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

			Parameter<std::filesystem::path, ParamType::In> Path{};
			Parameter<std::shared_ptr<data::Mesh>, ParamType::Out> Mesh{};

			bool Invoke() override;
		};

	}
}
