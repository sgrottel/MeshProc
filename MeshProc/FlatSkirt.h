#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class FlatSkirt : public AbstractCommand
	{
	public:
		FlatSkirt(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

		inline void SetMesh(std::shared_ptr<data::Mesh>& mesh)
		{
			const_cast<std::shared_ptr<data::Mesh>&>(m_mesh) = mesh;
		}

		inline void SetLoop(std::shared_ptr<std::vector<uint32_t>>& loop)
		{
			const_cast<std::shared_ptr<std::vector<uint32_t>>&>(m_loop) = loop;
		}

		inline std::shared_ptr<std::vector<uint32_t>> GetNewLoop() const
		{
			return m_newLoop;
		}

		inline glm::vec3 GetCenter() const
		{
			return m_center;
		}

		inline glm::vec3 GetX2D() const
		{
			return m_x2D;
		}

		inline glm::vec3 GetY2D() const
		{
			return m_y2D;
		}

		inline glm::vec3 GetZDir() const
		{
			return m_zDir;
		}

		inline float GetZDist() const
		{
			return m_zDist;
		}

	private:
		std::shared_ptr<data::Mesh> m_mesh;
		const std::shared_ptr<std::vector<uint32_t>> m_loop;
		std::shared_ptr<std::vector<uint32_t>> m_newLoop;
		glm::vec3 m_center;
		glm::vec3 m_x2D;
		glm::vec3 m_y2D;
		glm::vec3 m_zDir;
		float m_zDist;
	};
}
