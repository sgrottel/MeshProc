#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"
#include "data/Scene.h"

#include <glm/glm.hpp>

namespace meshproc
{

	class PlaceMesh : public AbstractCommand
	{
	public:
		PlaceMesh(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		std::shared_ptr<data::Scene> m_scene;
		const std::shared_ptr<data::Mesh> m_mesh;
		const glm::mat4 m_mat{ 1 };
	};
}
