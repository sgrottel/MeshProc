#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class ExtractSubMesh : public AbstractCommand
	{
	public:
		ExtractSubMesh(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<data::Mesh> m_inputMesh;
		std::shared_ptr<data::Mesh> m_outputMesh;
		const std::shared_ptr<std::vector<uint32_t>> m_triList;
	};

}
