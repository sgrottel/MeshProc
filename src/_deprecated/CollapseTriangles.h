#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <random>
#include <memory>
#include <vector>

namespace meshproc
{

	class CollapseTriangles : public AbstractCommand
	{
	public:
		CollapseTriangles(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		std::shared_ptr<data::Mesh> m_mesh{};
		const std::shared_ptr<std::vector<uint32_t>> m_tris{};
	};

}
