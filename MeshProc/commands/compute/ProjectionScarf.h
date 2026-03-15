#pragma once

#include "commands/AbstractCommand.h"

#include "data/Mesh.h"
#include "data/HalfSpace.h"

#include <memory>
#include <vector>

namespace meshproc
{
	namespace commands
	{
		namespace compute
		{

			class ProjectionScarf : public AbstractCommand
			{
			public:
				ProjectionScarf(const sgrottel::ISimpleLog& log);

				bool Invoke() override;

			private:
				struct Mesh2D;

				std::vector<uint32_t> SelectTriangles();
				Mesh2D ProjectTriangles(std::vector<uint32_t> const& selTris, glm::vec3 const& x2d, glm::vec3 const& y2d);

				const std::shared_ptr<data::Mesh> m_mesh;
				const std::shared_ptr<data::HalfSpace> m_projection;
				std::shared_ptr<data::Mesh> m_outmesh;

			};

		}
	}
}
