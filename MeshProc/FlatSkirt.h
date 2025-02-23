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

		Parameter<std::shared_ptr<data::Mesh>, ParamType::InOut> Mesh;
		Parameter<std::vector<uint32_t>, ParamType::In> Loop;
		Parameter<std::vector<uint32_t>, ParamType::Out> NewLoop;
		Parameter<glm::vec3, ParamType::Out> Center;
		Parameter<glm::vec3, ParamType::Out> X2D;
		Parameter<glm::vec3, ParamType::Out> Y2D;
		Parameter<glm::vec3, ParamType::Out> ZDir;
		Parameter<float, ParamType::Out> ZDist;

		bool Invoke() override;
	};
}
