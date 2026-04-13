#pragma once

#include "AbstractCommand.h"

#include <memory>

namespace meshproc
{
	namespace data
	{
		class Mesh;
		class Shape2D;
	}

	namespace generator
	{

		class RotateExtrude2DMesh : public AbstractCommand
		{
		public:
			RotateExtrude2DMesh(const sgrottel::ISimpleLog& log);
			bool Invoke() override;

		private:
			const float m_minAngle{ 0.0f }; // in Deg
			const float m_maxAngle{ 360.0f }; // in Deg
			const uint32_t m_steps{ 32 };
			const std::shared_ptr<data::Shape2D> m_shape2D;
			std::shared_ptr<data::Mesh> m_mesh;
			//std::shared_ptr<std::vector<uint32_t>> m_startLoop;
			//std::shared_ptr<std::vector<uint32_t>> m_endLoop;
			// TODO: add options to generate caps
		};

	}
}
