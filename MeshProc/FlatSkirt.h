#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class FlatSkirt : public AbstractCommand
	{
	public:
		FlatSkirt(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<data::Mesh> m_mesh;
		const std::shared_ptr<std::vector<uint32_t>> m_loop;
		std::shared_ptr<std::vector<uint32_t>> m_newLoop;
		glm::vec3 m_center;
		glm::vec3 m_x2D;
		glm::vec3 m_y2D;
		glm::vec3 m_zDir;
		float m_zDist;
	};
}
