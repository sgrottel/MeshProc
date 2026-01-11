#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>

namespace meshproc
{
	namespace generator
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
