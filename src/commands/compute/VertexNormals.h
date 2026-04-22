#pragma once

#include "commands/AbstractCommand.h"
#include "data/Mesh.h"

#include <glm/glm.hpp>

#include <memory>

namespace meshproc
{
	namespace commands
	{
		namespace compute
		{

			class VertexNormals : public AbstractCommand
			{
			public:
				VertexNormals(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				const std::shared_ptr<data::Mesh> m_mesh;
				std::shared_ptr<std::vector<glm::vec3>> m_normals;
			};

		}
	}
}
