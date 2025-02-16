#pragma once

#include "Mesh.h"

#include <memory>
#include <vector>

namespace sgrottel
{
	class ISimpleLog;
}

class OpenBorder
{
public:
	OpenBorder(sgrottel::ISimpleLog& log);

	std::vector<std::vector<uint32_t>> Find(std::shared_ptr<Mesh> const& mesh);

private:
	sgrottel::ISimpleLog& m_log;
};
