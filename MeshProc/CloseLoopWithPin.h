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

		inline void SetMesh(std::shared_ptr<data::Mesh>& mesh)
		{
			const_cast<std::shared_ptr<data::Mesh>&>(m_mesh) = mesh;
		}

		inline void SetLoop(std::shared_ptr<std::vector<uint32_t>>& loop)
		{
			const_cast<std::shared_ptr<std::vector<uint32_t>>&>(m_loop) = loop;
		}

		inline uint32_t GetNewVertexIndex() const
		{
			return m_newVertexIndex;
		}

	private:
		std::shared_ptr<data::Mesh> m_mesh;
		const std::shared_ptr<std::vector<uint32_t>> m_loop;
		uint32_t m_newVertexIndex;

	};

}
