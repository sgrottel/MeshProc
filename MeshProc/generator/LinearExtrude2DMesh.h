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

		class LinearExtrude2DMesh : public AbstractCommand
		{
		public:
			LinearExtrude2DMesh(const sgrottel::ISimpleLog& log);
			bool Invoke() override;

		private:
			const float m_minZ{ 0.0f };
			const float m_maxZ{ 1.0f };
			const std::shared_ptr<data::Shape2D> m_shape2D;
			std::shared_ptr<data::Mesh> m_mesh;
		};

	}
}
