#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class LinearExtrude : public AbstractCommand
	{
	public:
		LinearExtrude(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		std::shared_ptr<data::Mesh> m_mesh;
		std::shared_ptr<std::vector<uint32_t>> m_loop;
		const glm::vec3 m_dir{ 1.0f, 0.0f, 0.0f };
	};
}
