#include "CmdLineArgs.h"

#include <SimpleLog/SimpleLog.hpp>

#include <iostream>

int wmain(int argc, wchar_t **argv)
{
	sgrottel::NullLog nullLog;
	sgrottel::EchoingSimpleLog log{ nullLog };

	CmdLineArgs cmdLine;
	if (!cmdLine.Parse(log, argc, argv)) {
		return 1;
	}

	// TODO: Implement

	return 0;
}
