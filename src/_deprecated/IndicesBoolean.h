#pragma once

#include "AbstractCommand.h"

#include <memory>
#include <vector>

namespace meshproc
{

	// Selects all faces completely flat at the bottom (minimum z)
	// returns the indices of faces
	class IndicesBoolean : public AbstractCommand
	{
	public:
		IndicesBoolean(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<std::vector<uint32_t>> m_a{};
		const std::shared_ptr<std::vector<uint32_t>> m_b{};
		std::shared_ptr<std::vector<uint32_t>> m_res{};
		const uint32_t m_op{ 0 };
	};

}
