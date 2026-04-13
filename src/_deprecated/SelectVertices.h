#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class SelectVertices : public AbstractCommand
	{
	public:
		SelectVertices(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<data::Mesh> m_mesh;
		const std::shared_ptr<lua::CallbackFunction> m_callback;
		std::shared_ptr<std::vector<uint32_t>> m_selection;
	};
}
