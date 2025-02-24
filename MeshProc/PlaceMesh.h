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
		PlaceMesh(sgrottel::ISimpleLog& log);

		Parameter<std::shared_ptr<data::Scene>, ParamType::InOut> Scene;
		Parameter<std::shared_ptr<data::Mesh>, ParamType::In> Mesh;
		Parameter<glm::mat4, ParamType::In> Mat;

		bool Invoke() override;
	};
}
