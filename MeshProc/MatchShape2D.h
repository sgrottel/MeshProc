#pragma once

#include "AbstractCommand.h"
#include "data/Shape2D.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>

namespace meshproc
{
	class MatchShape2D : public AbstractCommand
	{
	public:
		MatchShape2D(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<data::Shape2D> m_shape{};
		const std::shared_ptr<data::Shape2D> m_shapeTarget{};
		glm::mat4 m_transform{};
	};
}
