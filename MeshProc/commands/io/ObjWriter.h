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

			class ObjWriter : public AbstractCommand
			{
			public:
				ObjWriter(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::wstring m_path{};
				const std::shared_ptr<data::Scene> m_scene{};
				const std::shared_ptr<std::vector<std::shared_ptr<std::vector<glm::vec3>>>> m_vertexColors{};
			};

		}
	}
}
