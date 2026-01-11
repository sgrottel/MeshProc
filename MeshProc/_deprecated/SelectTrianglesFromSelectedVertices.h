#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <random>
#include <memory>
#include <vector>

namespace meshproc
{

	// Selects all faces completely flat at the bottom (minimum z)
	// returns the indices of faces
	class SelectTrianglesFromSelectedVertices : public AbstractCommand
	{
	public:
		SelectTrianglesFromSelectedVertices(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<data::Mesh> m_mesh{};
		const std::shared_ptr<std::vector<uint32_t>> m_vertSel{};
		const uint32_t m_reqVertSel{ 3 };
		std::shared_ptr<std::vector<uint32_t>> m_trisSel{};
	};

}
