#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class ManipulateVertices : public AbstractCommand
	{
	public:
		ManipulateVertices(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		std::shared_ptr<data::Mesh> m_mesh;
		const std::shared_ptr<lua::CallbackFunction> m_callback;
		//std::shared_ptr<std::vector<uint32_t>> m_loop;
		//const glm::vec3 m_dir{ 1.0f, 0.0f, 0.0f };
	};
}
