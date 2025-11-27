#pragma once

#include "AbstractCommand.h"

#include "data/Mesh.h"

#include <glm/glm.hpp>

#include <random>
#include <memory>
#include <vector>

namespace meshproc
{

	class DisplacementNoise : public AbstractCommand
	{
	public:
		DisplacementNoise(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		std::shared_ptr<data::Mesh> m_mesh{};
		const std::shared_ptr<std::vector<glm::vec3>> m_dirs{};
		const float m_min{ 0.0f };
		const float m_max{ 1.0f };
		const uint32_t m_seed{ std::random_device{}() };
	};

}
