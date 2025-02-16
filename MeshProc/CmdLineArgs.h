#pragma once

#include <string>
#include <filesystem>
#include <vector>

namespace sgrottel
{
	class ISimpleLog;
}

class CmdLineArgs
{
public:
	std::vector<std::filesystem::path> inputs;

	bool Parse(sgrottel::ISimpleLog& log, int argc, wchar_t const* const* argv);
};
