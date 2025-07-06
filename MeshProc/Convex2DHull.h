#pragma once

#include "AbstractCommand.h"

#include "data/Shape2D.h"

namespace meshproc
{

	class Convex2DHull : public AbstractCommand
	{
	public:
		Convex2DHull(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<data::Shape2D> m_loops;
		std::shared_ptr<data::Shape2D> m_hull;
	};

}

