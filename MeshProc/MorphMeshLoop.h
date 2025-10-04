#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class MorphMeshLoop : public AbstractCommand
	{
	public:
		MorphMeshLoop(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		std::shared_ptr<data::Mesh> m_mesh;
		const std::shared_ptr<std::vector<uint32_t>> m_edgeList;
		const float m_blendArea{ 1.0f };
		const std::shared_ptr<data::Mesh> m_targetMesh;
		const std::shared_ptr<std::vector<uint32_t>> m_targetEdgeList;
	};

}
