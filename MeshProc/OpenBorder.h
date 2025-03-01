#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class OpenBorder : public AbstractCommand
	{
	public:
		OpenBorder(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

		inline void SetMesh(std::shared_ptr<data::Mesh> mesh)
		{
			const_cast<std::shared_ptr<data::Mesh>&>(m_mesh) = mesh;
		}

		inline std::shared_ptr<std::vector<std::shared_ptr<std::vector<uint32_t>>>> GetEdgeLists() const
		{
			return m_edgeLists;
		}

	private:
		const std::shared_ptr<data::Mesh> m_mesh;
		std::shared_ptr<std::vector<std::shared_ptr<std::vector<uint32_t>>>> m_edgeLists;
	};

}