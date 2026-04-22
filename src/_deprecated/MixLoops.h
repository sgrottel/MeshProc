#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class MixLoops : public AbstractCommand
	{
	public:
		MixLoops(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<data::Mesh> m_meshA;
		const std::shared_ptr<std::vector<uint32_t>> m_edgeListA;
		const std::shared_ptr<data::Mesh> m_meshB;
		const std::shared_ptr<std::vector<uint32_t>> m_edgeListB;
		const float m_mixFactor{ 0.5f };
		std::shared_ptr<data::Mesh> m_outMesh;
		std::shared_ptr<std::vector<uint32_t>> m_outEdgeList;
	};

}
