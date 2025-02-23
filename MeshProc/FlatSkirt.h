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

		Parameter<std::shared_ptr<data::Mesh>> Mesh; // inout
		Parameter<std::vector<uint32_t>> Loop; // in
		Parameter<std::vector<uint32_t>> NewLoop; // out
		Parameter<glm::vec3> Center; // out
		Parameter<glm::vec3> X2D; // out
		Parameter<glm::vec3> Y2D; // out
		Parameter<glm::vec3> ZDir; // out
		Parameter<float> ZDist; // out

		bool Invoke() override;
	};
}
