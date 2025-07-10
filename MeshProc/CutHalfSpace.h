#pragma once

#include "AbstractCommand.h"

#include "data/Mesh.h"
#include "data/Shape2D.h"

namespace meshproc
{

	class CutHalfSpace : public AbstractCommand
	{
	public:
		CutHalfSpace(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		std::shared_ptr<data::Mesh> m_mesh;
		const glm::vec3 m_planeNormal;
		const float m_planeDist;
		std::shared_ptr<std::vector<std::shared_ptr<std::vector<uint32_t>>>> m_openLoops;
	};

}
