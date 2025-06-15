#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	// Selects all faces completely flat at the bottom (minimum z)
	// returns the indices of faces
	class SelectBottomTriangles : public AbstractCommand
	{
	public:
		SelectBottomTriangles(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

		inline void SetMesh(std::shared_ptr<data::Mesh> mesh)
		{
			const_cast<std::shared_ptr<data::Mesh>&>(m_mesh) = mesh;
		}

		inline std::shared_ptr<std::vector<uint32_t>> GetTrianglesList() const
		{
			return m_triList;
		}

	private:
		const std::shared_ptr<data::Mesh> m_mesh;
		std::shared_ptr<std::vector<uint32_t>> m_triList;
	};

}
