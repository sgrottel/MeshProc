#pragma once

#include "AbstractCommand.h"

#include "data/Mesh.h"

#include <glm/glm.hpp>

namespace meshproc
{

	class MeasureBoundingBox : public AbstractCommand
	{
	public:
		MeasureBoundingBox(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<data::Mesh> m_mesh{};
		glm::vec3 m_min{ 0.0f };
		glm::vec3 m_max{ 1.0f };
	};

}
