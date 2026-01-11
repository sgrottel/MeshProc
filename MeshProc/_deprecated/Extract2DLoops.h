#pragma once

#include "AbstractCommand.h"

#include "data/Mesh.h"
#include "data/Shape2D.h"
#include "data/HalfSpace.h"

namespace meshproc
{

	class Extract2DLoops : public AbstractCommand
	{
	public:
		Extract2DLoops(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		const std::shared_ptr<data::Mesh> m_mesh;
		data::HalfSpace m_halfSpace;
		std::shared_ptr<data::Shape2D> m_loops;
	};

}
