#pragma once

#include <string>

namespace sgrottel
{
	class ISimpleLog;
}

class CmdLineArgs
{
public:

	bool Parse(sgrottel::ISimpleLog& log, int argc, wchar_t const* const* argv);
};
