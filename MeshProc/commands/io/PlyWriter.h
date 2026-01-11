#pragma once

#include "commands/AbstractCommand.h"
#include "data/Scene.h"

#include <filesystem>
#include <memory>

namespace meshproc
{
	namespace commands
	{
		namespace io
		{

			class PlyWriter : public AbstractCommand
			{
			public:
				PlyWriter(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::wstring m_path{};
				const std::shared_ptr<data::Scene> m_scene{};
			};

		}
	}
}
