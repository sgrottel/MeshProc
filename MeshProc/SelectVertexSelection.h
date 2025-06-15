#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class SelectVertexSelection : public AbstractCommand
	{
	public:
		SelectVertexSelection(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const uint32_t m_index{ 0 };
		const std::shared_ptr<std::vector<std::shared_ptr<std::vector<uint32_t>>>> m_lists;
		std::shared_ptr<std::vector<uint32_t>> m_list;
	};

}