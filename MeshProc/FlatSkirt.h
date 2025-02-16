#pragma once

#include "Mesh.h"

#include <memory>
#include <vector>

namespace sgrottel
{
	class ISimpleLog;
}

class FlatSkirt
{
public:
	FlatSkirt(sgrottel::ISimpleLog& log);

	std::vector<uint32_t> AddSkirt(std::shared_ptr<Mesh>& mesh, std::vector<uint32_t> const& loop);

	inline glm::vec3 const& GetCenter() const noexcept
	{
		return m_center;
	}
	inline glm::vec3 const& GetX2D() const noexcept
	{
		return m_x2d;
	}
	inline glm::vec3 const& GetY2D() const noexcept
	{
		return m_y2d;
	}

private:
	sgrottel::ISimpleLog& m_log;

	glm::vec3 m_center{};
	glm::vec3 m_x2d{ 1.0f, 0.0f, 0.0f };
	glm::vec3 m_y2d{ 0.0f, 1.0f, 0.0f };
};
