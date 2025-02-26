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
			ObjReader(const sgrottel::ISimpleLog& log);

			Parameter<std::filesystem::path, ParamMode::In> Path{};
			Parameter<std::shared_ptr<data::Mesh>, ParamMode::Out> Mesh{};

			bool Invoke() override;
		};

	}
}
