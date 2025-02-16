#pragma once

#include <string>
#include <filesystem>

namespace sgrottel
{
	class ISimpleLog;
}

class CmdLineArgs
{
public:
	std::filesystem::path input;

	bool Parse(sgrottel::ISimpleLog& log, int argc, wchar_t const* const* argv);
};
