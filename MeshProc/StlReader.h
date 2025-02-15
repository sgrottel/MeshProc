#pragma once

#include "Mesh.h"

#include <memory>
#include <filesystem>

namespace sgrottel
{
	class ISimpleLog;
}

class StlReader
{
public:
	StlReader(sgrottel::ISimpleLog& log);

	std::shared_ptr<Mesh> Load(std::filesystem::path const& file);

private:
	sgrottel::ISimpleLog& m_log;
};

