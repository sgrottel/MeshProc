#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"
#include "data/Scene.h"

#include <glm/glm.hpp>

namespace meshproc
{

	class PlaceMesh : public AbstractCommand
	{
	public:
		PlaceMesh(const sgrottel::ISimpleLog& log);

		Parameter<std::shared_ptr<data::Scene>, ParamMode::InOut> Scene;
		Parameter<std::shared_ptr<data::Mesh>, ParamMode::In> Mesh;
		Parameter<glm::mat4, ParamMode::In> Mat;

		bool Invoke() override;
	};
}
