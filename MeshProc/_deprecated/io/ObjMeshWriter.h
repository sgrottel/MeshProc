#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>
#include <vector>

namespace meshproc
{
	namespace io
	{

		class ObjMeshWriter : public AbstractCommand
		{
		public:
			ObjMeshWriter(const sgrottel::ISimpleLog& log);

			bool Invoke() override;

		private:
			const std::wstring m_path{};
			const std::shared_ptr<data::Mesh> m_mesh{};
			const std::shared_ptr<std::vector<glm::vec3>> m_vertColors{};
		};

	}
}
