#include "CmdLineArgs.h"
#include "CommandFactory.h"
#include "CommandRegistration.h"
#include "DevPlayground.h"
#include "lua/Runner.h"

#include <SimpleLog/SimpleLog.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <memory>
#include <cwctype>

int wmain(int argc, wchar_t **argv)
{
	using meshproc::CmdLineArgs;
	using meshproc::CliCommand;

	sgrottel::NullLog nullLog;
	sgrottel::EchoingSimpleLog log{ nullLog };

	CmdLineArgs cmdLine;
	if (!cmdLine.Parse(log, argc, argv)) {
		return (cmdLine.m_command == CliCommand::Error)
				? 1
				: 0;
	}

	log.SetEchoDetails(cmdLine.m_verbose);

	meshproc::CommandFactory cmdFactory{ log };
	meshproc::CommandRegistration(cmdFactory, log);

	switch (cmdLine.m_command)
	{
	case CliCommand::RunScript:
	{
		meshproc::lua::Runner lua{ log, cmdFactory };
		if (!lua.Init()) break;
		if (!lua.RegisterCommands()) break;

		log.Detail(L"Loading Lua: %s", cmdLine.m_script.wstring().c_str());
		if (!lua.LoadScript(cmdLine.m_script)) break;

		if (!lua.RunScript()) break;

		log.Message("done.");
	}
	break;

	case CliCommand::ListCommands:
		log.Message("");
		log.Message("Available commands:");
		cmdFactory.ListCommands(cmdLine.m_verbose);
		break;

	default:
		log.Critical("CLI command %d not implemented", cmdLine.m_command);
		break;
	}

	return 0;
}
