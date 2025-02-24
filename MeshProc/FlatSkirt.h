#pragma once

#include "AbstractCommand.h"
#include "data/Mesh.h"

#include <memory>
#include <vector>

namespace meshproc
{

	class FlatSkirt : public AbstractCommand
	{
	public:
		FlatSkirt(sgrottel::ISimpleLog& log);

		Parameter<std::shared_ptr<data::Mesh>, ParamMode::InOut> Mesh;
		Parameter<std::vector<uint32_t>, ParamMode::In> Loop;
		Parameter<std::vector<uint32_t>, ParamMode::Out> NewLoop;
		Parameter<glm::vec3, ParamMode::Out> Center;
		Parameter<glm::vec3, ParamMode::Out> X2D;
		Parameter<glm::vec3, ParamMode::Out> Y2D;
		Parameter<glm::vec3, ParamMode::Out> ZDir;
		Parameter<float, ParamMode::Out> ZDist;

		bool Invoke() override;
	};
}
