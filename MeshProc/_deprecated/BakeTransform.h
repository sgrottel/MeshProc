#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class BakeTransform : public AbstractCommand
	{
	public:
		BakeTransform(const sgrottel::ISimpleLog& log);

		bool Invoke() override;

	private:
		std::shared_ptr<data::Mesh> m_mesh;
		const glm::mat4 m_matrix{ 1.0 };
	};
};

