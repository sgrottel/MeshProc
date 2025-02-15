#pragma once

#include "Mesh.h"

#include <memory>
#include <vector>

namespace sgrottel
{
	class ISimpleLog;
}

class FlatSkirt
{
public:
	FlatSkirt(sgrottel::ISimpleLog& log);

	std::vector<uint32_t> AddSkirt(std::shared_ptr<Mesh>& mesh, std::vector<uint32_t> const& loop);

private:
	sgrottel::ISimpleLog& m_log;
};
