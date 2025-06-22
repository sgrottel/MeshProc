#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"
namespace meshproc
{

	class CloseLoopWithPin : public AbstractCommand
	{
	public:
		CloseLoopWithPin(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		std::shared_ptr<data::Mesh> m_mesh;
		const std::shared_ptr<std::vector<uint32_t>> m_loop;
		uint32_t m_newVertexIndex;
		glm::vec3 m_offset{0.0f, 0.0f, 0.0f};
	};

}
